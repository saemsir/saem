#include "cos_journal.h"

#include <stdalign.h>
#include <string.h>

#include "cos_atomic.h"
#include "cos_le.h"
#include "cos_sha256.h"
#include "cos_wal.h"

#define SB_MAGIC 0x4c4e4a53u  /* bytes 'S' 'J' 'N' 'L' */
#define REC_MAGIC 0x43455253u /* bytes 'S' 'R' 'E' 'C' */
#define VERSION 1u
#define SB_HASH_OFF 56u

struct CosJournal {
  alignas(COS_CACHE_LINE) _Atomic uint32_t lock; /* 0 free, 1 held; a full 32-bit word, never a byte (thesis 5.1: no byte as an atomic flag) */
  alignas(COS_CACHE_LINE) _Atomic uint64_t contention;
  CosBlockDev dev;
  CosWal *wal;
  _Atomic uint64_t next_seq; /* records stored; written only under the lock */
  uint64_t capacity;
  uint8_t broken; /* set after a device failure: refuse appends until the journal is reopened (which recovers) */
  uint8_t last_chain[32];
};

static size_t align_up(size_t n, size_t a) { return (n + a - 1u) / a * a; }

size_t cos_journal_bytes(void) { return align_up(sizeof(struct CosJournal), COS_CACHE_LINE) + cos_wal_bytes() + COS_CACHE_LINE; }

/* One writer at a time. true = the lock is now ours; every true must be paired with release().
 * Many harts: a loser spins until the lock is free (never returns false). The swap is one amoswap.w, where the old atomic_flag byte needed a
 * word-sized lr/sc loop (no byte AMO without Zabha).
 * One hart (COS_SINGLE_HART): whoever holds the lock can only be this same hart further up the call stack (a block-device callback or an
 * interrupt handler that called back in). Waiting for yourself is a hang, so the call is refused (false) and the caller returns COS_E_BUSY
 * without touching anything; it must NOT release, the lock belongs to the outer call. */
static bool acquire(CosJournal *j) {
  if (cos_exchange(&j->lock, 1u, memory_order_acquire) == 0u) return true;
#if COS_SINGLE_HART
  return false;
#else
  cos_fetch_add(&j->contention, 1u, memory_order_relaxed);
  while (cos_exchange(&j->lock, 1u, memory_order_acquire) != 0u) {
  }
  return true;
#endif
}
static void release(CosJournal *j) { cos_store(&j->lock, 0u, memory_order_release); }

static void genesis(uint8_t out[32]) { cos_sha256("SAEM-JOURNAL-V1", 15, out); }

static void sb_build(const CosJournal *j, uint64_t next_seq, const uint8_t chain[32], uint8_t out[COS_BLOCK_SIZE]) {
  memset(out, 0, COS_BLOCK_SIZE);
  cos_put32(out, SB_MAGIC);
  cos_put32(out + 4, VERSION);
  cos_put32(out + 8, j->dev.nblocks);
  cos_put32(out + 12, COS_HOME_START);
  cos_put64(out + 16, next_seq);
  memcpy(out + 24, chain, 32);
  cos_sha256(out, SB_HASH_OFF, out + SB_HASH_OFF);
}

static void rec_build(uint8_t out[COS_BLOCK_SIZE], uint16_t kind, const void *payload, uint16_t len, uint64_t seq, uint64_t tick,
                      const uint8_t prev[32]) {
  memset(out, 0, COS_BLOCK_SIZE);
  cos_put32(out, REC_MAGIC);
  cos_put16(out + 4, kind);
  cos_put16(out + 6, len);
  cos_put64(out + 8, seq);
  cos_put64(out + 16, tick);
  memcpy(out + 24, prev, 32);
  if (len > 0u) memcpy(out + COS_REC_HEADER, payload, len);
  CosSha256 c; /* chain = SHA-256(header bytes before the chain field, then the payload) */
  cos_sha256_init(&c);
  cos_sha256_update(&c, out, 56);
  cos_sha256_update(&c, out + COS_REC_HEADER, len);
  cos_sha256_final(&c, out + 56);
}

static CosStatus rec_parse(const uint8_t b[COS_BLOCK_SIZE], CosRecord *r) {
  if (cos_get32(b) != REC_MAGIC) return COS_E_CORRUPT;
  const uint16_t len = cos_get16(b + 6);
  if (len > COS_PAYLOAD_MAX) return COS_E_CORRUPT;
  CosSha256 c;
  uint8_t h[32];
  cos_sha256_init(&c);
  cos_sha256_update(&c, b, 56);
  cos_sha256_update(&c, b + COS_REC_HEADER, len);
  cos_sha256_final(&c, h);
  if (!cos_ct_equal(h, b + 56, 32)) return COS_E_CORRUPT;
  memset(r, 0, sizeof *r);
  r->kind = cos_get16(b + 4);
  r->len = len;
  r->seq = cos_get64(b + 8);
  r->tick = cos_get64(b + 16);
  memcpy(r->prev, b + 24, 32);
  memcpy(r->chain, b + 56, 32);
  memcpy(r->payload, b + COS_REC_HEADER, len);
  return COS_OK;
}

static CosStatus read_record(CosJournal *j, uint64_t seq, CosRecord *r) {
  uint8_t b[COS_BLOCK_SIZE];
  if (j->dev.read(j->dev.ctx, (uint32_t)(COS_HOME_START + seq), b) != 0) return COS_E_IO;
  const CosStatus st = rec_parse(b, r);
  if (st != COS_OK) return st;
  return r->seq == seq ? COS_OK : COS_E_CORRUPT;
}

CosStatus cos_journal_open(void *mem, size_t len, const CosBlockDev *dev, bool format, CosJournal **out) {
  if (out == NULL) return COS_E_INVALID;
  *out = NULL;
  if (mem == NULL || dev == NULL || len < cos_journal_bytes() || ((uintptr_t)mem % COS_CACHE_LINE) != 0u) return COS_E_ALIGN;
  if (dev->nblocks < COS_HOME_START + 1u) return COS_E_INVALID;
  memset(mem, 0, sizeof(struct CosJournal));
  CosJournal *j = (CosJournal *)mem;
  cos_init(&j->lock, 0u);
  cos_init(&j->contention, 0u);
  cos_init(&j->next_seq, 0u);
  j->dev = *dev;
  j->capacity = (uint64_t)dev->nblocks - COS_HOME_START;
  uint8_t *wal_mem = (uint8_t *)mem + align_up(sizeof(struct CosJournal), COS_CACHE_LINE);
  j->wal = cos_wal_init(wal_mem, len - (size_t)(wal_mem - (uint8_t *)mem), dev, 1u);
  if (j->wal == NULL) return COS_E_INVALID;
  uint32_t replayed = 0;
  CosStatus st = cos_wal_recover(j->wal, &replayed);
  if (st != COS_OK) return st;
  uint8_t sb[COS_BLOCK_SIZE];
  if (format) {
    genesis(j->last_chain);
    sb_build(j, 0u, j->last_chain, sb);
    cos_wal_begin(j->wal);
    st = cos_wal_put(j->wal, 0u, sb);
    if (st == COS_OK) st = cos_wal_commit(j->wal);
    if (st != COS_OK) return st;
  } else {
    if (j->dev.read(j->dev.ctx, 0u, sb) != 0) return COS_E_IO;
    if (cos_get32(sb) != SB_MAGIC) return COS_E_NOT_FOUND; /* never formatted */
    uint8_t h[32];
    cos_sha256(sb, SB_HASH_OFF, h);
    if (cos_get32(sb + 4) != VERSION || !cos_ct_equal(h, sb + SB_HASH_OFF, 32)) return COS_E_CORRUPT;
    if (cos_get32(sb + 12) != COS_HOME_START || cos_get32(sb + 8) > dev->nblocks) return COS_E_CORRUPT;
    const uint64_t n = cos_get64(sb + 16);
    if (n > j->capacity) return COS_E_CORRUPT;
    cos_store(&j->next_seq, n, memory_order_relaxed);
    memcpy(j->last_chain, sb + 24, 32);
    if (n > 0u) { /* the newest stored record must be the one the superblock names */
      CosRecord r;
      st = read_record(j, n - 1u, &r);
      if (st != COS_OK) return st;
      if (!cos_ct_equal(r.chain, j->last_chain, 32)) return COS_E_CORRUPT;
    } else {
      uint8_t g[32];
      genesis(g);
      if (!cos_ct_equal(g, j->last_chain, 32)) return COS_E_CORRUPT;
    }
  }
  *out = j;
  return COS_OK;
}

CosStatus cos_journal_append(CosJournal *j, uint16_t kind, const void *payload, uint16_t len, uint64_t tick) {
  if (len > COS_PAYLOAD_MAX) return COS_E_TOO_BIG;
  if (len > 0u && payload == NULL) return COS_E_INVALID;
  CosStatus st = COS_OK;
  if (!acquire(j)) return COS_E_BUSY;
  const uint64_t seq = cos_load(&j->next_seq, memory_order_relaxed);
  if (j->broken) {
    st = COS_E_IO;
  } else if (seq >= j->capacity) {
    st = COS_E_FULL;
  } else {
    uint8_t rb[COS_BLOCK_SIZE], sb[COS_BLOCK_SIZE];
    rec_build(rb, kind, payload, len, seq, tick, j->last_chain);
    sb_build(j, seq + 1u, rb + 56, sb);
    cos_wal_begin(j->wal);
    st = cos_wal_put(j->wal, (uint32_t)(COS_HOME_START + seq), rb);
    if (st == COS_OK) st = cos_wal_put(j->wal, 0u, sb);
    if (st == COS_OK) st = cos_wal_commit(j->wal);
    if (st == COS_OK) {
      memcpy(j->last_chain, rb + 56, 32);
      cos_store(&j->next_seq, seq + 1u, memory_order_release);
    } else {
      cos_wal_abort(j->wal);
      if (st == COS_E_IO) j->broken = 1u; /* the medium is in an unknown state until reopened and recovered */
    }
  }
  release(j);
  return st;
}

CosStatus cos_journal_read(CosJournal *j, uint64_t seq, CosRecord *out) {
  if (!acquire(j)) return COS_E_BUSY;
  CosStatus st = seq < cos_load(&j->next_seq, memory_order_relaxed) ? read_record(j, seq, out) : COS_E_NOT_FOUND;
  release(j);
  return st;
}

CosStatus cos_journal_verify(CosJournal *j, uint64_t *good, uint64_t *bad_seq) {
  CosStatus st = COS_OK;
  if (!acquire(j)) { /* nothing was checked: report "unverified from the start", never "all good" */
    if (good != NULL) *good = 0u;
    if (bad_seq != NULL) *bad_seq = 0u;
    return COS_E_BUSY;
  }
  const uint64_t n = cos_load(&j->next_seq, memory_order_relaxed);
  uint8_t running[32];
  genesis(running);
  uint64_t s = 0;
  for (; s < n; ++s) {
    CosRecord r;
    st = read_record(j, s, &r);
    if (st == COS_OK && !cos_ct_equal(r.prev, running, 32)) st = COS_E_CORRUPT;
    if (st != COS_OK) break;
    memcpy(running, r.chain, 32);
  }
  if (st == COS_OK && !cos_ct_equal(running, j->last_chain, 32)) st = COS_E_CORRUPT;
  release(j);
  if (good != NULL) *good = s;
  if (bad_seq != NULL) *bad_seq = (st == COS_OK) ? UINT64_MAX : s;
  return st;
}

uint64_t cos_journal_count(const CosJournal *j) { return cos_load(&j->next_seq, memory_order_acquire); }
uint64_t cos_journal_capacity(const CosJournal *j) { return j->capacity; }
uint64_t cos_journal_contention(const CosJournal *j) { return cos_load(&j->contention, memory_order_relaxed); }

void cos_journal_head(CosJournal *j, uint8_t out[32]) {
  if (!acquire(j)) { /* refused: all zero bytes, which no chain value is, so any comparison with a real anchor fails closed */
    memset(out, 0, 32);
    return;
  }
  memcpy(out, j->last_chain, 32);
  release(j);
}
