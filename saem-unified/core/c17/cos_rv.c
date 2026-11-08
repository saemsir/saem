/* QCOS assembly layer, part 1: encode, decode and format one RISC-V instruction word. See cos_rv.h for the contract.
 * Field layouts: RISC-V Unprivileged ISA (project copy riscv-unprivileged.pdf) chapter 2.2 "Base Instruction Formats" and 2.3 "Immediate Encoding Variants";
 * Hennessy and Patterson (RISC-V edition, project copy HandP_RISCV(1)) figure 2.19. The opcode values are NOT here: they come from cos_rv_tab.c (generated). */
#include "cos_rv.h"

#include <string.h>

static bool isa_ok(const CosRvIsa *isa) {
  return isa != NULL && (isa->xlen == 32u || isa->xlen == 64u) && (isa->exts & ~COS_RV_EXT_ALL) == 0u;
}

static bool row_on(const CosRvIsa *isa, const CosRvRow *r) {
  const unsigned xbit = isa->xlen == 64u ? 2u : 1u;
  return (r->xlens & xbit) != 0u && (r->ext == 0u || (isa->exts & r->ext) != 0u);
}

const CosRvRow *cos_rv_row_of(const CosRvIsa *isa, uint8_t id) {
  if (!isa_ok(isa)) return NULL;
  for (size_t i = 0; i < COS_RV_ROWS; i++) {
    if (cos_rv_rows[i].id == id && row_on(isa, &cos_rv_rows[i])) return &cos_rv_rows[i];
  }
  return NULL;
}

const CosRvRow *cos_rv_row_by_name(const CosRvIsa *isa, const char *name, size_t len) {
  if (!isa_ok(isa) || name == NULL || len == 0u || len >= sizeof cos_rv_rows[0].name) return NULL;
  for (size_t i = 0; i < COS_RV_ROWS; i++) {
    const CosRvRow *r = &cos_rv_rows[i];
    if (row_on(isa, r) && strlen(r->name) == len && memcmp(r->name, name, len) == 0) return r;
  }
  return NULL;
}

/* sign-extend the low `bits` bits of v */
static int32_t sx(uint32_t v, unsigned bits) {
  const uint32_t m = 1u << (bits - 1u);
  v &= (m << 1) - 1u;
  return (int32_t)((v ^ m) - m);
}

CosStatus cos_rv_encode(const CosRvIsa *isa, const CosRvInsn *in, uint32_t *word) {
  if (!isa_ok(isa) || in == NULL || word == NULL) return COS_E_INVALID;
  const CosRvRow *r = cos_rv_row_of(isa, in->id);
  if (r == NULL) return COS_E_NOT_FOUND;
  uint32_t w = r->match;
  const int32_t imm = in->imm;
  const uint32_t rd = in->rd, rs1 = in->rs1, rs2 = in->rs2;
  switch ((CosRvCls)r->cls) {
    case COS_RV_CLS_R:
      if ((rd | rs1 | rs2) > 31u) return COS_E_INVALID;
      w |= rd << 7 | rs1 << 15 | rs2 << 20;
      break;
    case COS_RV_CLS_I:
    case COS_RV_CLS_L:
    case COS_RV_CLS_JR:
      if ((rd | rs1) > 31u) return COS_E_INVALID;
      if (imm < -2048 || imm > 2047) return COS_E_RANGE;
      w |= rd << 7 | rs1 << 15 | ((uint32_t)imm & 0xFFFu) << 20;
      break;
    case COS_RV_CLS_SH5:
    case COS_RV_CLS_SH6:
      if ((rd | rs1) > 31u) return COS_E_INVALID;
      if (imm < 0 || imm > (r->cls == (uint8_t)COS_RV_CLS_SH5 ? 31 : 63)) return COS_E_RANGE;
      w |= rd << 7 | rs1 << 15 | (uint32_t)imm << 20;
      break;
    case COS_RV_CLS_S:
      if ((rs1 | rs2) > 31u) return COS_E_INVALID;
      if (imm < -2048 || imm > 2047) return COS_E_RANGE;
      w |= rs1 << 15 | rs2 << 20 | (((uint32_t)imm >> 5) & 0x7Fu) << 25 | ((uint32_t)imm & 0x1Fu) << 7;
      break;
    case COS_RV_CLS_B: {
      if ((rs1 | rs2) > 31u) return COS_E_INVALID;
      if (imm < -4096 || imm > 4094 || (imm & 1) != 0) return COS_E_RANGE;
      const uint32_t u = (uint32_t)imm;
      w |= rs1 << 15 | rs2 << 20 | ((u >> 12) & 1u) << 31 | ((u >> 5) & 0x3Fu) << 25 | ((u >> 1) & 0xFu) << 8 | ((u >> 11) & 1u) << 7;
      break;
    }
    case COS_RV_CLS_U:
      if (rd > 31u) return COS_E_INVALID;
      if (imm < 0 || imm > 0xFFFFF) return COS_E_RANGE;
      w |= rd << 7 | (uint32_t)imm << 12;
      break;
    case COS_RV_CLS_J: {
      if (rd > 31u) return COS_E_INVALID;
      if (imm < -1048576 || imm > 1048574 || (imm & 1) != 0) return COS_E_RANGE;
      const uint32_t u = (uint32_t)imm;
      w |= rd << 7 | ((u >> 20) & 1u) << 31 | ((u >> 1) & 0x3FFu) << 21 | ((u >> 11) & 1u) << 20 | ((u >> 12) & 0xFFu) << 12;
      break;
    }
    case COS_RV_CLS_AMO:
      if ((rd | rs1 | rs2) > 31u || (in->aq | in->rl) > 1u) return COS_E_INVALID;
      w |= rd << 7 | rs1 << 15 | rs2 << 20 | (uint32_t)in->rl << 25 | (uint32_t)in->aq << 26;
      break;
    case COS_RV_CLS_LR:
      if ((rd | rs1) > 31u || (in->aq | in->rl) > 1u) return COS_E_INVALID;
      w |= rd << 7 | rs1 << 15 | (uint32_t)in->rl << 25 | (uint32_t)in->aq << 26;
      break;
    case COS_RV_CLS_CSR:
    case COS_RV_CLS_CSRI:
    case COS_RV_CLS_FENCE:
    case COS_RV_CLS_FENCEI:
      if ((rd | rs1) > 31u) return COS_E_INVALID;
      if (imm < 0 || imm > 4095) return COS_E_RANGE;
      w |= rd << 7 | rs1 << 15 | (uint32_t)imm << 20;
      break;
    case COS_RV_CLS_N:
      break;
    case COS_RV_CLS_COUNT:
    default:
      return COS_E_INVALID;
  }
  *word = w;
  return COS_OK;
}

CosStatus cos_rv_decode(const CosRvIsa *isa, uint32_t w, CosRvInsn *out) {
  if (!isa_ok(isa) || out == NULL) return COS_E_INVALID;
  if ((w & 3u) != 3u) return COS_E_INVALID; /* a 16-bit form: not handled here */
  for (size_t i = 0; i < COS_RV_ROWS; i++) {
    const CosRvRow *r = &cos_rv_rows[i];
    if ((w & r->mask) != r->match || !row_on(isa, r)) continue;
    CosRvInsn d;
    memset(&d, 0, sizeof d);
    d.id = r->id;
    const uint8_t rd = (uint8_t)((w >> 7) & 31u), rs1 = (uint8_t)((w >> 15) & 31u), rs2 = (uint8_t)((w >> 20) & 31u);
    switch ((CosRvCls)r->cls) {
      case COS_RV_CLS_R:
        d.rd = rd, d.rs1 = rs1, d.rs2 = rs2;
        break;
      case COS_RV_CLS_I:
      case COS_RV_CLS_L:
      case COS_RV_CLS_JR:
        d.rd = rd, d.rs1 = rs1, d.imm = sx(w >> 20, 12);
        break;
      case COS_RV_CLS_SH5:
        d.rd = rd, d.rs1 = rs1, d.imm = (int32_t)((w >> 20) & 31u);
        break;
      case COS_RV_CLS_SH6:
        d.rd = rd, d.rs1 = rs1, d.imm = (int32_t)((w >> 20) & 63u);
        break;
      case COS_RV_CLS_S:
        d.rs1 = rs1, d.rs2 = rs2, d.imm = sx(((w >> 25) << 5) | ((w >> 7) & 31u), 12);
        break;
      case COS_RV_CLS_B:
        d.rs1 = rs1, d.rs2 = rs2;
        d.imm = sx(((w >> 31) & 1u) << 12 | ((w >> 7) & 1u) << 11 | ((w >> 25) & 0x3Fu) << 5 | ((w >> 8) & 0xFu) << 1, 13);
        break;
      case COS_RV_CLS_U:
        d.rd = rd, d.imm = (int32_t)(w >> 12);
        break;
      case COS_RV_CLS_J:
        d.rd = rd;
        d.imm = sx(((w >> 31) & 1u) << 20 | ((w >> 12) & 0xFFu) << 12 | ((w >> 20) & 1u) << 11 | ((w >> 21) & 0x3FFu) << 1, 21);
        break;
      case COS_RV_CLS_AMO:
        d.rd = rd, d.rs1 = rs1, d.rs2 = rs2, d.aq = (uint8_t)((w >> 26) & 1u), d.rl = (uint8_t)((w >> 25) & 1u);
        break;
      case COS_RV_CLS_LR:
        d.rd = rd, d.rs1 = rs1, d.aq = (uint8_t)((w >> 26) & 1u), d.rl = (uint8_t)((w >> 25) & 1u);
        break;
      case COS_RV_CLS_CSR:
      case COS_RV_CLS_CSRI:
      case COS_RV_CLS_FENCE:
      case COS_RV_CLS_FENCEI:
        d.rd = rd, d.rs1 = rs1, d.imm = (int32_t)(w >> 20);
        break;
      case COS_RV_CLS_N:
        break;
      case COS_RV_CLS_COUNT:
      default:
        return COS_E_INVALID;
    }
    *out = d;
    return COS_OK;
  }
  return COS_E_INVALID;
}

bool cos_rv_canonical(const CosRvIsa *isa, const CosRvInsn *in) {
  if (in == NULL) return false;
  const CosRvRow *r = cos_rv_row_of(isa, in->id);
  if (r == NULL) return false;
  if (r->cls == (uint8_t)COS_RV_CLS_FENCE) {
    return in->rd == 0u && in->rs1 == 0u && (in->imm < 0x100 || in->imm == 0x833) && in->imm >= 0;
  }
  if (r->cls == (uint8_t)COS_RV_CLS_FENCEI) return in->rd == 0u && in->rs1 == 0u && in->imm == 0;
  return true;
}

/* ------------------------------------------------------------------ text */
typedef struct {
  char *p;
  size_t cap, n;
  bool bad;
} Out;

static void put(Out *o, char c) {
  if (o->n + 1u < o->cap) o->p[o->n] = c;
  else o->bad = true;
  o->n++;
}
static void put_s(Out *o, const char *s) {
  while (*s != '\0') put(o, *s++);
}
static void put_dec(Out *o, int32_t v) {
  char t[12];
  unsigned k = 0;
  uint32_t u = (uint32_t)v;
  if (v < 0) {
    put(o, '-');
    u = 0u - u;
  }
  do {
    t[k++] = (char)('0' + (u % 10u));
    u /= 10u;
  } while (u != 0u);
  while (k != 0u) put(o, t[--k]);
}
static void put_hex(Out *o, uint32_t u) {
  char t[8];
  unsigned k = 0;
  put_s(o, "0x");
  do {
    t[k++] = "0123456789abcdef"[u & 15u];
    u >>= 4;
  } while (u != 0u);
  while (k != 0u) put(o, t[--k]);
}
static void put_rel(Out *o, int32_t off) {
  put(o, '.');
  put(o, off < 0 ? '-' : '+');
  put_dec(o, off < 0 ? (int32_t)(0u - (uint32_t)off) : off);
}

static const char RN[32][5] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
                               "s2",   "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

const char *cos_rv_reg_name(unsigned r) { return r < 32u ? RN[r] : "?"; }

const char *cos_rv_csr_name(unsigned xlen, unsigned addr) {
  const unsigned bit = xlen == 32u ? 1u : (xlen == 64u ? 2u : 0u);
  for (size_t i = 0; i < COS_RV_CSRS; i++) {
    if (cos_rv_csr_names[i].addr == addr && (cos_rv_csr_names[i].xlens & bit) != 0u) return cos_rv_csr_names[i].name;
  }
  return NULL;
}

static void put_reg(Out *o, unsigned r) { put_s(o, cos_rv_reg_name(r)); }
static void put_fence(Out *o, unsigned a) {
  if (a == 0u) {
    put(o, '0');
    return;
  }
  if ((a & 8u) != 0u) put(o, 'i');
  if ((a & 4u) != 0u) put(o, 'o');
  if ((a & 2u) != 0u) put(o, 'r');
  if ((a & 1u) != 0u) put(o, 'w');
}

size_t cos_rv_format(const CosRvIsa *isa, const CosRvInsn *in, char *buf, size_t cap) {
  if (buf == NULL || cap == 0u) return 0u;
  buf[0] = '\0';
  if (in == NULL) return 0u;
  const CosRvRow *r = cos_rv_row_of(isa, in->id);
  if (r == NULL) return 0u;
  Out o = {buf, cap, 0u, false};
  const unsigned cls = r->cls;
  if (cls == (uint8_t)COS_RV_CLS_FENCE && in->imm == 0x833 && in->rd == 0u && in->rs1 == 0u) {
    put_s(&o, "fence.tso");
  } else {
    put_s(&o, r->name);
    if (cls == (uint8_t)COS_RV_CLS_AMO || cls == (uint8_t)COS_RV_CLS_LR) {
      if (in->aq != 0u && in->rl != 0u) put_s(&o, ".aqrl");
      else if (in->aq != 0u) put_s(&o, ".aq");
      else if (in->rl != 0u) put_s(&o, ".rl");
    }
    switch ((CosRvCls)cls) {
      case COS_RV_CLS_R:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_reg(&o, in->rs1), put_s(&o, ", "), put_reg(&o, in->rs2);
        break;
      case COS_RV_CLS_I:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_reg(&o, in->rs1), put_s(&o, ", "), put_dec(&o, in->imm);
        break;
      case COS_RV_CLS_SH5:
      case COS_RV_CLS_SH6:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_reg(&o, in->rs1), put_s(&o, ", "), put_dec(&o, in->imm);
        break;
      case COS_RV_CLS_L:
      case COS_RV_CLS_JR:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_dec(&o, in->imm), put(&o, '('), put_reg(&o, in->rs1), put(&o, ')');
        break;
      case COS_RV_CLS_S:
        put(&o, ' '), put_reg(&o, in->rs2), put_s(&o, ", "), put_dec(&o, in->imm), put(&o, '('), put_reg(&o, in->rs1), put(&o, ')');
        break;
      case COS_RV_CLS_B:
        put(&o, ' '), put_reg(&o, in->rs1), put_s(&o, ", "), put_reg(&o, in->rs2), put_s(&o, ", "), put_rel(&o, in->imm);
        break;
      case COS_RV_CLS_U:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_hex(&o, (uint32_t)in->imm);
        break;
      case COS_RV_CLS_J:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_rel(&o, in->imm);
        break;
      case COS_RV_CLS_AMO:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", "), put_reg(&o, in->rs2), put_s(&o, ", ("), put_reg(&o, in->rs1), put(&o, ')');
        break;
      case COS_RV_CLS_LR:
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", ("), put_reg(&o, in->rs1), put(&o, ')');
        break;
      case COS_RV_CLS_CSR:
      case COS_RV_CLS_CSRI: {
        const char *cn = cos_rv_csr_name(isa->xlen, (unsigned)in->imm);
        put(&o, ' '), put_reg(&o, in->rd), put_s(&o, ", ");
        if (cn != NULL) put_s(&o, cn);
        else put_hex(&o, (uint32_t)in->imm);
        put_s(&o, ", ");
        if (cls == (uint8_t)COS_RV_CLS_CSR) put_reg(&o, in->rs1);
        else put_dec(&o, (int32_t)in->rs1);
        break;
      }
      case COS_RV_CLS_FENCE:
        if (((uint32_t)in->imm >> 8) != 0u) { /* a form the specification reserves: shown, not assemblable */
          put_s(&o, ".fm"), put_hex(&o, (uint32_t)in->imm >> 8);
        }
        put(&o, ' '), put_fence(&o, ((uint32_t)in->imm >> 4) & 15u), put_s(&o, ", "), put_fence(&o, (uint32_t)in->imm & 15u);
        break;
      case COS_RV_CLS_FENCEI:
      case COS_RV_CLS_N:
      case COS_RV_CLS_COUNT:
      default:
        break;
    }
  }
  if (o.bad) {
    buf[0] = '\0';
    return 0u;
  }
  buf[o.n] = '\0';
  return o.n;
}
