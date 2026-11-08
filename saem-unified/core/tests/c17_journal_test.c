/* The journal: chained records, tamper detection, reopen, capacity, atomic append at every power-cut point, and concurrent writers.
 * Built with -DCOS_SINGLE_HART=1 (the monitor-core profile, see cos_atomic.h) the threads are replaced by the re-entry test: a block-device
 * callback that calls back into the journal is refused with COS_E_BUSY, never waited for, and the outer call is unharmed. */
#include <stdalign.h>
#include <string.h>

#include "../c17/cos_atomic.h"
#include "../c17/cos_journal.h"
#include "../c17/cos_sha256.h"
#include "c17_check.h"
#include "c17_ramdev.h"
#if !COS_SINGLE_HART
#include <pthread.h>
#endif

#define NB 128u
static uint8_t img[NB * COS_BLOCK_SIZE];
static uint8_t base[NB * COS_BLOCK_SIZE];
#if !COS_SINGLE_HART
static uint8_t big[400u * COS_BLOCK_SIZE]; /* room for the 150-record threaded run */
#endif
static alignas(64) uint8_t jmem[40000];
static alignas(64) uint8_t jmem2[40000];

static CosJournal *open_j(RamDev *d, bool format, CosStatus *st) {
  CosBlockDev dev = ram_dev(d);
  CosJournal *j = NULL;
  *st = cos_journal_open(jmem, sizeof jmem, &dev, format, &j);
  return j;
}

#if !COS_SINGLE_HART
typedef struct Arg {
  CosJournal *j;
  unsigned id;
  unsigned errors;
} Arg;

static void *writer(void *p) {
  Arg *a = (Arg *)p;
  for (unsigned i = 0; i < 50; ++i) {
    uint8_t pl[2] = {(uint8_t)a->id, (uint8_t)i};
    CosStatus st;
    do { st = cos_journal_append(a->j, 9, pl, 2, i); } while (st == COS_E_FULL);
    if (st != COS_OK) a->errors++;
  }
  return NULL;
}
#else
/* A block device that calls back into the journal on every write (what a buggy driver or an interrupt handler that "just logs" would do). */
typedef struct Reenter {
  RamDev *inner;
  CosJournal *j;
  bool armed;
  unsigned tries;  /* nested calls made */
  unsigned busy;   /* nested calls answered COS_E_BUSY with the "nothing done" output convention */
} Reenter;

static int re_read(void *c, uint32_t blk, uint8_t *buf) { return ram_read(((Reenter *)c)->inner, blk, buf); }
static int re_flush(void *c) { return ram_flush(((Reenter *)c)->inner); }
static int re_write(void *c, uint32_t blk, const uint8_t *buf) {
  Reenter *r = (Reenter *)c;
  if (r->armed) {
    static const uint8_t zero[32];
    CosRecord rec;
    uint64_t good = 77u, badseq = 77u;
    uint8_t h[32];
    memset(h, 0xAA, sizeof h);
    r->tries += 4u;
    r->busy += (cos_journal_append(r->j, 1, "x", 1, 0) == COS_E_BUSY);
    r->busy += (cos_journal_read(r->j, 0, &rec) == COS_E_BUSY);
    r->busy += (cos_journal_verify(r->j, &good, &badseq) == COS_E_BUSY && good == 0u && badseq == 0u); /* "unverified from the start", never "all good" */
    cos_journal_head(r->j, h);
    r->busy += (memcmp(h, zero, 32) == 0); /* all zero: no real chain value, so any comparison with an anchor fails closed */
  }
  return ram_write(r->inner, blk, buf);
}
#endif

int main(void) {
  CosSuite s = {"c17_journal", 0, 0};
  RamDev d = {img, NB, -1, 0, false, 0, 0};
  CosStatus st;
  CosJournal *j;

  CK(&s, cos_journal_bytes() <= sizeof jmem);
  /* a blank device was never formatted */
  memset(img, 0, sizeof img);
  j = open_j(&d, false, &st);
  CK(&s, j == NULL && st == COS_E_NOT_FOUND);

  /* format, append, read back */
  j = open_j(&d, true, &st);
  CK(&s, st == COS_OK && j != NULL);
  CK(&s, cos_journal_count(j) == 0u && cos_journal_capacity(j) == NB - COS_HOME_START);
  uint8_t g[32], head[32];
  cos_sha256("SAEM-JOURNAL-V1", 15, g);
  cos_journal_head(j, head);
  CK(&s, memcmp(g, head, 32) == 0); /* an empty journal's head is the genesis value */
  uint8_t pay[COS_PAYLOAD_MAX];
  for (unsigned i = 0; i < sizeof pay; ++i) pay[i] = (uint8_t)(i * 31u + 7u);
  const uint16_t lens[5] = {0, 1, 100, COS_PAYLOAD_MAX, 17};
  for (unsigned i = 0; i < 5; ++i) CK(&s, cos_journal_append(j, (uint16_t)(10 + i), pay, lens[i], 1000u + i) == COS_OK);
  CK(&s, cos_journal_append(j, 1, pay, COS_PAYLOAD_MAX + 1u, 0) == COS_E_TOO_BIG);
  CK(&s, cos_journal_append(j, 1, NULL, 3, 0) == COS_E_INVALID);
  CK(&s, cos_journal_count(j) == 5u);
  for (unsigned i = 0; i < 5; ++i) {
    CosRecord r;
    CK(&s, cos_journal_read(j, i, &r) == COS_OK && r.seq == i && r.kind == 10u + i && r.len == lens[i] && r.tick == 1000u + i);
    CK(&s, memcmp(r.payload, pay, lens[i]) == 0);
    if (i > 0u) { CosRecord q; CK(&s, cos_journal_read(j, i - 1u, &q) == COS_OK && memcmp(q.chain, r.prev, 32) == 0); }
  }
  CosRecord rr;
  CK(&s, cos_journal_read(j, 5, &rr) == COS_E_NOT_FOUND);
  uint64_t good = 0, bad = 0;
  CK(&s, cos_journal_verify(j, &good, &bad) == COS_OK && good == 5u);
  cos_journal_head(j, head);

  /* reopen from the same medium */
  j = open_j(&d, false, &st);
  CK(&s, st == COS_OK && cos_journal_count(j) == 5u);
  uint8_t head2[32];
  cos_journal_head(j, head2);
  CK(&s, memcmp(head, head2, 32) == 0);
  CK(&s, cos_journal_append(j, 20, "more", 4, 2000) == COS_OK && cos_journal_append(j, 21, "more", 4, 2001) == COS_OK);
  CK(&s, cos_journal_verify(j, &good, &bad) == COS_OK && good == 7u);
  memcpy(base, img, sizeof img);

  /* tamper: a payload byte of record 2 */
  img[(size_t)(COS_HOME_START + 2u) * COS_BLOCK_SIZE + COS_REC_HEADER + 3u] ^= 1u;
  j = open_j(&d, false, &st); /* the newest record is intact, so opening works... */
  CK(&s, st == COS_OK && j != NULL);
  CK(&s, cos_journal_verify(j, &good, &bad) == COS_E_CORRUPT && bad == 2u && good == 2u); /* ...and verify names the bad record */
  CK(&s, cos_journal_read(j, 2, &rr) == COS_E_CORRUPT);
  memcpy(img, base, sizeof img);

  /* tamper: the superblock, and the newest record */
  img[20] ^= 0x10;
  j = open_j(&d, false, &st);
  CK(&s, j == NULL && st == COS_E_CORRUPT);
  memcpy(img, base, sizeof img);
  img[(size_t)(COS_HOME_START + 6u) * COS_BLOCK_SIZE + 30u] ^= 0x80;
  j = open_j(&d, false, &st);
  CK(&s, j == NULL && st == COS_E_CORRUPT);
  memcpy(img, base, sizeof img);

  /* tamper: swap two whole valid records */
  { uint8_t t[COS_BLOCK_SIZE];
    uint8_t *a = img + (size_t)(COS_HOME_START + 1u) * COS_BLOCK_SIZE, *b = img + (size_t)(COS_HOME_START + 2u) * COS_BLOCK_SIZE;
    memcpy(t, a, COS_BLOCK_SIZE); memcpy(a, b, COS_BLOCK_SIZE); memcpy(b, t, COS_BLOCK_SIZE); }
  j = open_j(&d, false, &st);
  CK(&s, st == COS_OK && cos_journal_verify(j, &good, &bad) == COS_E_CORRUPT && bad == 1u);
  memcpy(img, base, sizeof img);

  /* capacity: a full journal is an answer */
  { RamDev small = {img, COS_HOME_START + 4u, -1, 0, false, 0, 0};
    memset(img, 0, sizeof img);
    CosJournal *k = open_j(&small, true, &st);
    CK(&s, st == COS_OK && cos_journal_capacity(k) == 4u);
    for (unsigned i = 0; i < 4; ++i) CK(&s, cos_journal_append(k, 1, "x", 1, i) == COS_OK);
    CK(&s, cos_journal_append(k, 1, "x", 1, 9) == COS_E_FULL && cos_journal_count(k) == 4u);
    CK(&s, cos_journal_verify(k, &good, &bad) == COS_OK); }

  /* every power-cut point inside the 3rd append (6 writes), lost / half-landed / sliver */
  memset(img, 0, sizeof img);
  d.nblocks = NB; d.budget = -1; d.dead = false;
  j = open_j(&d, true, &st);
  CK(&s, cos_journal_append(j, 1, "a", 1, 1) == COS_OK && cos_journal_append(j, 1, "b", 1, 2) == COS_OK);
  memcpy(base, img, sizeof img);
  for (int tear = 0; tear <= 2; ++tear) {
    for (long k = 0; k <= 6; ++k) {
      memcpy(img, base, sizeof img);
      ram_reboot(&d);
      d.writes = 0; d.tear = tear; d.budget = k;
      j = open_j(&d, false, &st);
      CK(&s, st == COS_OK);
      const CosStatus ast = cos_journal_append(j, 1, "c", 1, 3);
      CK(&s, (k < 6) ? ast != COS_E_FULL && ast != COS_OK : ast == COS_OK);
      if (k < 6) CK(&s, cos_journal_append(j, 1, "d", 1, 4) == COS_E_IO); /* a broken journal refuses more until reopened */
      ram_reboot(&d);
      j = open_j(&d, false, &st);
      CK(&s, st == COS_OK && j != NULL);
      const uint64_t n = cos_journal_count(j);
      CK(&s, n == 2u || n == 3u);
      /* the commit header is the 3rd of the 6 writes: lost before it, complete after it; a half-landed header may go either way */
      if (k >= 3) CK(&s, n == 3u);
      else if (k == 2 && tear == 1) CK(&s, n == 2u || n == 3u);
      else CK(&s, n == 2u);
      CK(&s, cos_journal_verify(j, &good, &bad) == COS_OK && good == n);
      CK(&s, cos_journal_append(j, 1, "e", 1, 5) == COS_OK && cos_journal_verify(j, &good, &bad) == COS_OK); /* and it keeps working */
    }
  }

#if !COS_SINGLE_HART
  /* three writers at once: nothing lost, nothing twice, each writer's order kept */
  memset(big, 0, sizeof big);
  RamDev bd = {big, 400u, -1, 0, false, 0, 0};
  { CosBlockDev dev = ram_dev(&bd);
    CosJournal *t = NULL;
    CK(&s, cos_journal_open(jmem2, sizeof jmem2, &dev, true, &t) == COS_OK);
    Arg a[3]; pthread_t th[3];
    for (unsigned i = 0; i < 3; ++i) { a[i].j = t; a[i].id = i; a[i].errors = 0; pthread_create(&th[i], NULL, writer, &a[i]); }
    unsigned errs = 0;
    for (unsigned i = 0; i < 3; ++i) { pthread_join(th[i], NULL); errs += a[i].errors; }
    CK(&s, errs == 0u && cos_journal_count(t) == 150u);
    CK(&s, cos_journal_verify(t, &good, &bad) == COS_OK && good == 150u);
    unsigned next[3] = {0, 0, 0};
    int order_ok = 1;
    for (uint64_t q = 0; q < 150u; ++q) {
      CosRecord r;
      if (cos_journal_read(t, q, &r) != COS_OK || r.len != 2u || r.payload[0] > 2u) { order_ok = 0; break; }
      if (r.payload[1] != next[r.payload[0]]++) order_ok = 0;
    }
    CK(&s, order_ok && next[0] == 50u && next[1] == 50u && next[2] == 50u);
    printf("  threads: 150 appends, %llu waits for the writer lock\n", (unsigned long long)cos_journal_contention(t)); }
#else
  /* one hart: a callback that calls back in is refused (COS_E_BUSY), the outer append still lands, and the lock is released exactly once */
  memset(img, 0, sizeof img);
  { RamDev rd = {img, NB, -1, 0, false, 0, 0};
    Reenter re = {&rd, NULL, false, 0, 0};
    CosBlockDev dev = {&re, NB, re_read, re_write, re_flush};
    CosJournal *t = NULL;
    CK(&s, cos_journal_open(jmem2, sizeof jmem2, &dev, true, &t) == COS_OK);
    re.j = t;
    re.armed = true;
    CK(&s, cos_journal_append(t, 1, "outer", 5, 1) == COS_OK);
    re.armed = false;
    CK(&s, re.tries >= 8u && re.busy == re.tries); /* every nested call of every write was refused, none waited, none changed anything */
    CK(&s, cos_journal_count(t) == 1u);            /* only the outer record exists */
    CK(&s, cos_journal_verify(t, &good, &bad) == COS_OK && good == 1u); /* the lock was free again: this call is not BUSY */
    CK(&s, cos_journal_append(t, 1, "next", 4, 2) == COS_OK && cos_journal_count(t) == 2u);
    CK(&s, cos_journal_verify(t, &good, &bad) == COS_OK && good == 2u);
    printf("  re-entry: %u nested calls, %u refused with COS_E_BUSY, outer append intact\n", re.tries, re.busy); }
#endif
  return suite_done(&s);
}
