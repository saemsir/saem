/* The assembly layer: encode, decode and format one RISC-V instruction word (core/c17/cos_rv.c and the generated table cos_rv_tab.c).
 *
 * What is proven here, with no LLVM present (tools/check_rv.py proves the same equalities against llvm-mc and llvm-objdump over about 250,000 words per register width):
 *   1. known words with their canonical text. The words and texts come from llvm-mc 18.1.3 and agree with a hand calculation from the base formats
 *      (Hennessy and Patterson, RISC-V edition, figure 2.19; RISC-V Unprivileged ISA 2.2 and 2.3). A build without LLVM keeps them as its oracle.
 *   2. encodings from operands that were calculated by hand, independent of the decoder.
 *   3. the edge of every immediate range and the status of every refusal (a bad operand is refused, never truncated).
 *   4. the profile gate: the application profile has no CSR instruction, no mret, no wfi, no fence.i; RV64-only instructions do not exist in RV32.
 *   5. identities over a deterministic sweep: encode(decode(w)) = w, decode(encode(x)) = x, the name lookup finds the row.
 *   6. the formatter's buffer rules, the width-dependent CSR names, the reserved-field flag, the table's own well-formedness.
 * Erratum pinned by this file: the project copy HandP_RISCV(1) (text line about 6490) writes 0x67 as the branch opcode. The branch major opcode is 0x63; 0x67 is jalr. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../c17/cos_rv.h"
#include "c17_check.h"

typedef struct Vec {
  uint32_t word;
  const char *text;
  uint8_t widths; /* bit 0: the word is an instruction in RV32, bit 1: in RV64 */
} Vec;

static const Vec VEC[] = {
  {0x00000013u, "addi zero, zero, 0", 3u},
  {0x00150513u, "addi a0, a0, 1", 3u},
  {0x80050513u, "addi a0, a0, -2048", 3u},
  {0x7FF50513u, "addi a0, a0, 2047", 3u},
  {0x00C58533u, "add a0, a1, a2", 3u},
  {0x40C58533u, "sub a0, a1, a2", 3u},
  {0x00812503u, "lw a0, 8(sp)", 3u},
  {0x80012503u, "lw a0, -2048(sp)", 3u},
  {0x00A12423u, "sw a0, 8(sp)", 3u},
  {0xFEA12E23u, "sw a0, -4(sp)", 3u},
  {0x00B50463u, "beq a0, a1, .+8", 3u},
  {0xFEB51EE3u, "bne a0, a1, .-4", 3u},
  {0x7EB54FE3u, "blt a0, a1, .+4094", 3u},
  {0x80B57063u, "bgeu a0, a1, .-4096", 3u},
  {0x0400006Fu, "jal zero, .+64", 3u},
  {0x800000EFu, "jal ra, .-1048576", 3u},
  {0x7FFFF0EFu, "jal ra, .+1048574", 3u},
  {0x00008067u, "jalr zero, 0(ra)", 3u},
  {0x12345537u, "lui a0, 0x12345", 3u},
  {0xFFFFF537u, "lui a0, 0xfffff", 3u},
  {0x00001517u, "auipc a0, 0x1", 3u},
  {0x00000073u, "ecall", 3u},
  {0x00100073u, "ebreak", 3u},
  {0x30200073u, "mret", 3u},
  {0x10500073u, "wfi", 3u},
  {0x0000100Fu, "fence.i", 3u},
  {0x0330000Fu, "fence rw, rw", 3u},
  {0x8330000Fu, "fence.tso", 3u},
  {0x0840000Fu, "fence i, o", 3u},
  {0xF1402573u, "csrrs a0, mhartid, zero", 3u},
  {0x30051073u, "csrrw zero, mstatus, a0", 3u},
  {0x30045073u, "csrrwi zero, mstatus, 8", 3u},
  {0x304FF2F3u, "csrrci t0, mie, 31", 3u},
  {0x06B6252Fu, "amoadd.w.aqrl a0, a1, (a2)", 3u},
  {0x08B6252Fu, "amoswap.w a0, a1, (a2)", 3u},
  {0x1005A52Fu, "lr.w a0, (a1)", 3u},
  {0x1405A52Fu, "lr.w.aq a0, (a1)", 3u},
  {0x1AC5A52Fu, "sc.w.rl a0, a2, (a1)", 3u},
  {0x02C58533u, "mul a0, a1, a2", 3u},
  {0x02C5B533u, "mulhu a0, a1, a2", 3u},
  {0x02C5C533u, "div a0, a1, a2", 3u},
  {0x02C5F533u, "remu a0, a1, a2", 3u},
  {0x00351513u, "slli a0, a0, 3", 3u},
  {0x01F55513u, "srli a0, a0, 31", 3u},
  {0x40355513u, "srai a0, a0, 3", 3u},
  {0xFFF5B513u, "sltiu a0, a1, -1", 3u},
  {0xFFF5C513u, "xori a0, a1, -1", 3u},
  {0x0FF5F513u, "andi a0, a1, 255", 3u},
  {0x0015051Bu, "addiw a0, a0, 1", 2u},
  {0x00813503u, "ld a0, 8(sp)", 2u},
  {0x00A13423u, "sd a0, 8(sp)", 2u},
  {0x02151513u, "slli a0, a0, 33", 2u},
  {0x43F55513u, "srai a0, a0, 63", 2u},
  {0x00C5953Bu, "sllw a0, a1, a2", 2u},
  {0x00816503u, "lwu a0, 8(sp)", 2u},
  {0x00B6352Fu, "amoadd.d a0, a1, (a2)", 2u},
  {0x1605B52Fu, "lr.d.aqrl a0, (a1)", 2u},
  {0x02C5853Bu, "mulw a0, a1, a2", 2u},
  {0x02C5D53Bu, "divuw a0, a1, a2", 2u},
};

typedef struct Enc {
  uint8_t id, rd, rs1, rs2, aq, rl;
  int32_t imm;
  uint32_t want;
} Enc;

/* fields calculated by hand from the format figures: rd at bit 7, rs1 at 15, rs2 at 20, funct3 at 12 */
static const Enc ENC[] = {
    {COS_RV_ADDI, 10, 10, 0, 0, 0, 1, 0x00150513u},       /* addi a0, a0, 1 */
    {COS_RV_ADD, 10, 11, 12, 0, 0, 0, 0x00C58533u},       /* add a0, a1, a2 */
    {COS_RV_SUB, 10, 11, 12, 0, 0, 0, 0x40C58533u},       /* sub: bit 30 */
    {COS_RV_MUL, 10, 11, 12, 0, 0, 0, 0x02C58533u},       /* mul: funct7 = 1 */
    {COS_RV_LW, 10, 2, 0, 0, 0, 8, 0x00812503u},          /* lw a0, 8(sp) */
    {COS_RV_SW, 0, 2, 10, 0, 0, 8, 0x00A12423u},          /* sw a0, 8(sp): imm[4:0] at bit 7 */
    {COS_RV_SW, 0, 2, 10, 0, 0, -4, 0xFEA12E23u},         /* sw a0, -4(sp): imm[11:5] at bit 25 */
    {COS_RV_BEQ, 0, 10, 11, 0, 0, 8, 0x00B50463u},        /* beq a0, a1, +8: imm[4:1] at bit 8 */
    {COS_RV_BNE, 0, 10, 11, 0, 0, -4, 0xFEB51EE3u},       /* bne a0, a1, -4: sign at bit 31 */
    {COS_RV_JAL, 0, 0, 0, 0, 0, 64, 0x0400006Fu},         /* jal zero, +64: the first word of the QCOS image header */
    {COS_RV_JALR, 0, 1, 0, 0, 0, 0, 0x00008067u},         /* ret */
    {COS_RV_LUI, 10, 0, 0, 0, 0, 0x12345, 0x12345537u},   /* lui a0, 0x12345 */
    {COS_RV_AUIPC, 10, 0, 0, 0, 0, 1, 0x00001517u},       /* auipc a0, 1 */
    {COS_RV_CSRRS, 10, 0, 0, 0, 0, 0xF14, 0xF1402573u},   /* csrr a0, mhartid */
    {COS_RV_CSRRWI, 0, 8, 0, 0, 0, 0x300, 0x30045073u},   /* csrrwi zero, mstatus, 8: the 5-bit immediate sits in the rs1 field */
    {COS_RV_AMOADD_W, 10, 12, 11, 1, 1, 0, 0x06B6252Fu},  /* amoadd.w.aqrl a0, a1, (a2): aq at bit 26, rl at bit 25 */
    {COS_RV_LR_W, 10, 11, 0, 1, 0, 0, 0x1405A52Fu},       /* lr.w.aq a0, (a1) */
    {COS_RV_FENCE, 0, 0, 0, 0, 0, 0x033, 0x0330000Fu},    /* fence rw, rw */
    {COS_RV_FENCE, 0, 0, 0, 0, 0, 0x833, 0x8330000Fu},    /* fence.tso */
    {COS_RV_FENCE_I, 0, 0, 0, 0, 0, 0, 0x0000100Fu},
    {COS_RV_ECALL, 0, 0, 0, 0, 0, 0, 0x00000073u},
    {COS_RV_EBREAK, 0, 0, 0, 0, 0, 0, 0x00100073u},
    {COS_RV_MRET, 0, 0, 0, 0, 0, 0, 0x30200073u},
    {COS_RV_WFI, 0, 0, 0, 0, 0, 0, 0x10500073u},
};

typedef struct Edge {
  uint8_t id;
  int32_t imm;
  uint8_t widths; /* which register widths the case is run in: bit 0 RV32, bit 1 RV64 */
  uint8_t ok;     /* 1: COS_OK, 0: COS_E_RANGE */
} Edge;

static const Edge EDGE[] = {
    {COS_RV_ADDI, 2047, 3, 1},    {COS_RV_ADDI, -2048, 3, 1},     {COS_RV_ADDI, 2048, 3, 0},   {COS_RV_ADDI, -2049, 3, 0},
    {COS_RV_ADDI, INT32_MAX, 3, 0}, {COS_RV_ADDI, INT32_MIN, 3, 0}, {COS_RV_LW, 2047, 3, 1},     {COS_RV_LW, 2048, 3, 0},
    {COS_RV_JALR, -2048, 3, 1},   {COS_RV_JALR, -2049, 3, 0},     {COS_RV_SW, 2047, 3, 1},     {COS_RV_SW, -2048, 3, 1},
    {COS_RV_SW, 2048, 3, 0},      {COS_RV_SW, -2049, 3, 0},       {COS_RV_BEQ, 4094, 3, 1},    {COS_RV_BEQ, -4096, 3, 1},
    {COS_RV_BEQ, 4096, 3, 0},     {COS_RV_BEQ, -4098, 3, 0},      {COS_RV_BEQ, 1, 3, 0},       {COS_RV_BEQ, -1, 3, 0},
    {COS_RV_BEQ, 4095, 3, 0},     {COS_RV_JAL, 1048574, 3, 1},    {COS_RV_JAL, -1048576, 3, 1}, {COS_RV_JAL, 1048576, 3, 0},
    {COS_RV_JAL, -1048578, 3, 0}, {COS_RV_JAL, 1, 3, 0},          {COS_RV_LUI, 0, 3, 1},       {COS_RV_LUI, 0xFFFFF, 3, 1},
    {COS_RV_LUI, 0x100000, 3, 0}, {COS_RV_LUI, -1, 3, 0},         {COS_RV_SLLI, 31, 3, 1},     {COS_RV_SLLI, 32, 1, 0},
    {COS_RV_SLLI, 32, 2, 1},      {COS_RV_SLLI, 63, 2, 1},        {COS_RV_SLLI, 64, 2, 0},     {COS_RV_SLLI, -1, 3, 0},
    {COS_RV_SLLIW, 31, 2, 1},     {COS_RV_SLLIW, 32, 2, 0},       {COS_RV_CSRRW, 0, 3, 1},     {COS_RV_CSRRW, 4095, 3, 1},
    {COS_RV_CSRRW, 4096, 3, 0},   {COS_RV_CSRRW, -1, 3, 0},       {COS_RV_CSRRWI, 4095, 3, 1}, {COS_RV_CSRRWI, 4096, 3, 0},
    {COS_RV_FENCE, 4095, 3, 1},   {COS_RV_FENCE, 4096, 3, 0},
};

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define NOTE(s, before, ...)                  \
  do {                                        \
    if ((s)->failed != (before)) {            \
      fprintf(stderr, "      " __VA_ARGS__);  \
      fputc('\n', stderr);                    \
    }                                         \
  } while (0)

static const CosRvIsa RV32 = COS_RV_ISA_RV32;
static const CosRvIsa RV64 = COS_RV_ISA_RV64;

static bool same(const CosRvInsn *a, const CosRvInsn *b) {
  return a->id == b->id && a->rd == b->rd && a->rs1 == b->rs1 && a->rs2 == b->rs2 && a->aq == b->aq && a->rl == b->rl && a->imm == b->imm;
}

static uint32_t xs(uint32_t *x) { /* xorshift32: the same words on every machine */
  *x ^= *x << 13;
  *x ^= *x >> 17;
  *x ^= *x << 5;
  return *x;
}

/* ---------------------------------------------------------------- 1. known words and their text */
static void test_vectors(CosSuite *s) {
  const CosRvIsa isas[2] = {COS_RV_ISA_RV32, COS_RV_ISA_RV64};
  for (unsigned k = 0; k < 2u; k++) {
    for (size_t i = 0; i < COUNT(VEC); i++) {
      const Vec *v = &VEC[i];
      const int before = s->failed;
      CosRvInsn in;
      char text[64];
      uint32_t again = 0;
      const CosStatus st = cos_rv_decode(&isas[k], v->word, &in);
      if ((((unsigned)v->widths >> k) & 1u) == 0u) {
        CK(s, st == COS_E_INVALID); /* an RV64-only word is no instruction in RV32 */
        NOTE(s, before, "RV%u: 0x%08lx must not decode", isas[k].xlen, (unsigned long)v->word);
        continue;
      }
      CK(s, st == COS_OK);
      if (st != COS_OK) {
        NOTE(s, before, "RV%u: 0x%08lx (%s) does not decode", isas[k].xlen, (unsigned long)v->word, v->text);
        continue;
      }
      CK(s, cos_rv_canonical(&isas[k], &in));
      CK(s, cos_rv_format(&isas[k], &in, text, sizeof text) == strlen(v->text));
      CK(s, strcmp(text, v->text) == 0);
      CK(s, cos_rv_encode(&isas[k], &in, &again) == COS_OK && again == v->word);
      NOTE(s, before, "RV%u: 0x%08lx wants '%s', got '%s' (re-encoded 0x%08lx)", isas[k].xlen, (unsigned long)v->word, v->text, text, (unsigned long)again);
    }
  }
}

/* ---------------------------------------------------------------- 2. encodings from operands */
static void test_encode(CosSuite *s) {
  const CosRvIsa isas[2] = {COS_RV_ISA_RV32, COS_RV_ISA_RV64};
  for (unsigned k = 0; k < 2u; k++) {
    for (size_t i = 0; i < COUNT(ENC); i++) {
      const Enc *e = &ENC[i];
      const int before = s->failed;
      const CosRvInsn in = {e->id, e->rd, e->rs1, e->rs2, e->aq, e->rl, e->imm};
      uint32_t w = 0xDEADBEEFu;
      CK(s, cos_rv_encode(&isas[k], &in, &w) == COS_OK);
      CK(s, w == e->want);
      NOTE(s, before, "RV%u: encode #%u gives 0x%08lx, want 0x%08lx", isas[k].xlen, (unsigned)i, (unsigned long)w, (unsigned long)e->want);
    }
  }
  /* the major opcodes: branch 0x63 (not the 0x67 of the printed H&P text), jalr 0x67, jal 0x6F, load 0x03, store 0x23, op 0x33, op-imm 0x13, lui 0x37, auipc 0x17, system 0x73 */
  static const struct {
    uint8_t id;
    uint32_t opcode;
  } MAJOR[] = {{COS_RV_BEQ, 0x63u}, {COS_RV_BNE, 0x63u}, {COS_RV_BLT, 0x63u}, {COS_RV_BGE, 0x63u}, {COS_RV_BLTU, 0x63u}, {COS_RV_BGEU, 0x63u},
               {COS_RV_JALR, 0x67u}, {COS_RV_JAL, 0x6Fu}, {COS_RV_LW, 0x03u}, {COS_RV_SW, 0x23u}, {COS_RV_ADD, 0x33u}, {COS_RV_ADDI, 0x13u},
               {COS_RV_LUI, 0x37u}, {COS_RV_AUIPC, 0x17u}, {COS_RV_ECALL, 0x73u}, {COS_RV_FENCE, 0x0Fu}, {COS_RV_AMOADD_W, 0x2Fu}};
  for (size_t i = 0; i < COUNT(MAJOR); i++) {
    CosRvInsn in;
    memset(&in, 0, sizeof in);
    in.id = MAJOR[i].id;
    uint32_t w = 0;
    CK(s, cos_rv_encode(&RV32, &in, &w) == COS_OK);
    CK(s, (w & 0x7Fu) == MAJOR[i].opcode);
  }
}

/* ---------------------------------------------------------------- 3. range edges and refusals */
static void test_edges(CosSuite *s) {
  const CosRvIsa isas[2] = {COS_RV_ISA_RV32, COS_RV_ISA_RV64};
  for (size_t i = 0; i < COUNT(EDGE); i++) {
    const Edge *e = &EDGE[i];
    for (unsigned k = 0; k < 2u; k++) {
      if ((((unsigned)e->widths >> k) & 1u) == 0u) continue;
      const int before = s->failed;
      const CosRvInsn in = {e->id, 1, 2, 3, 0, 0, e->imm};
      uint32_t w = 0xDEADBEEFu;
      const CosStatus st = cos_rv_encode(&isas[k], &in, &w);
      if (e->ok != 0u) {
        CosRvInsn back;
        CK(s, st == COS_OK);
        CK(s, cos_rv_decode(&isas[k], w, &back) == COS_OK && back.imm == e->imm && back.id == e->id); /* the immediate comes back exactly */
      } else {
        CK(s, st == COS_E_RANGE);
        CK(s, w == 0xDEADBEEFu); /* nothing is written on refusal */
      }
      NOTE(s, before, "RV%u: edge #%u (id %u, imm %ld) gave status %d", isas[k].xlen, (unsigned)i, (unsigned)e->id, (long)e->imm, (int)st);
    }
  }
  /* register 32 and aq/rl 2 are refused in the fields a class uses and ignored in the fields it does not use */
  static const struct {
    uint8_t id;
    unsigned used; /* bit 0 rd, bit 1 rs1, bit 2 rs2 */
  } USE[] = {{COS_RV_ADD, 7}, {COS_RV_ADDI, 3}, {COS_RV_LW, 3}, {COS_RV_SW, 6}, {COS_RV_BEQ, 6}, {COS_RV_LUI, 1}, {COS_RV_JAL, 1},
             {COS_RV_AMOADD_W, 7}, {COS_RV_LR_W, 3}, {COS_RV_CSRRW, 3}, {COS_RV_SLLI, 3}};
  for (size_t i = 0; i < COUNT(USE); i++) {
    for (unsigned p = 0; p < 3u; p++) {
      CosRvInsn in = {USE[i].id, 1, 2, 3, 0, 0, 0};
      uint8_t *f = p == 0u ? &in.rd : (p == 1u ? &in.rs1 : &in.rs2);
      uint32_t w = 0;
      *f = 32;
      const CosStatus st = cos_rv_encode(&RV64, &in, &w);
      CK(s, st == (((USE[i].used >> p) & 1u) != 0u ? COS_E_INVALID : COS_OK));
      *f = 255;
      CK(s, cos_rv_encode(&RV64, &in, &w) == (((USE[i].used >> p) & 1u) != 0u ? COS_E_INVALID : COS_OK));
    }
  }
  CosRvInsn amo = {COS_RV_AMOADD_W, 1, 2, 3, 2, 0, 0};
  CosRvInsn lr = {COS_RV_LR_W, 1, 2, 0, 0, 2, 0};
  CosRvInsn add = {COS_RV_ADD, 1, 2, 3, 2, 2, 0};
  uint32_t w = 0;
  CK(s, cos_rv_encode(&RV32, &amo, &w) == COS_E_INVALID);
  CK(s, cos_rv_encode(&RV32, &lr, &w) == COS_E_INVALID);
  CK(s, cos_rv_encode(&RV32, &add, &w) == COS_OK); /* aq and rl mean nothing to add */
  /* ids that are no instruction */
  CosRvInsn nope = {COS_RV_OP_COUNT, 0, 0, 0, 0, 0, 0};
  CK(s, cos_rv_encode(&RV32, &nope, &w) == COS_E_NOT_FOUND);
  nope.id = 255;
  CK(s, cos_rv_encode(&RV32, &nope, &w) == COS_E_NOT_FOUND);
}

/* ---------------------------------------------------------------- 4. profiles and bad arguments */
static void test_profiles(CosSuite *s) {
  const CosRvIsa app32 = COS_RV_ISA_APP32, app64 = COS_RV_ISA_APP64;
  const CosRvIsa m_only = {32u, COS_RV_EXT_M}, a_only = {32u, COS_RV_EXT_A};
  const CosRvIsa bad_width = {16u, COS_RV_EXT_ALL}, bad_exts = {32u, 0x80u};
  CosRvInsn in;
  uint32_t w = 0xDEADBEEFu;
  memset(&in, 0, sizeof in);
  /* application profile: what the monitor accepts from a program. ecall and ebreak stay (the program must be able to ask and to stop), all privileged forms go */
  static const uint8_t GONE[] = {COS_RV_CSRRW, COS_RV_CSRRWI, COS_RV_MRET, COS_RV_WFI, COS_RV_FENCE_I};
  for (size_t i = 0; i < COUNT(GONE); i++) {
    in.id = GONE[i];
    CK(s, cos_rv_encode(&app32, &in, &w) == COS_E_NOT_FOUND);
    CK(s, cos_rv_encode(&app64, &in, &w) == COS_E_NOT_FOUND);
    CK(s, cos_rv_encode(&RV32, &in, &w) == COS_OK);
    CK(s, cos_rv_row_of(&app32, GONE[i]) == NULL && cos_rv_row_of(&RV32, GONE[i]) != NULL);
  }
  static const uint32_t PRIV[] = {0x30200073u, 0x10500073u, 0x0000100Fu, 0x30051073u, 0xF1402573u};
  for (size_t i = 0; i < COUNT(PRIV); i++) {
    CK(s, cos_rv_decode(&app32, PRIV[i], &in) == COS_E_INVALID);
    CK(s, cos_rv_decode(&app64, PRIV[i], &in) == COS_E_INVALID);
    CK(s, cos_rv_decode(&RV32, PRIV[i], &in) == COS_OK);
  }
  CK(s, cos_rv_decode(&app32, 0x00000073u, &in) == COS_OK && cos_rv_decode(&app32, 0x00100073u, &in) == COS_OK); /* ecall, ebreak */
  CK(s, cos_rv_decode(&app32, 0x02C58533u, &in) == COS_OK);                                                    /* mul: the M extension is on */
  CK(s, cos_rv_decode(&app32, 0x06B6252Fu, &in) == COS_OK);                                                    /* amoadd.w.aqrl: the A extension is on */
  CK(s, cos_rv_decode(&a_only, 0x02C58533u, &in) == COS_E_INVALID);                                            /* mul with M off */
  CK(s, cos_rv_decode(&m_only, 0x06B6252Fu, &in) == COS_E_INVALID);                                            /* amo with A off */
  CK(s, cos_rv_decode(&a_only, 0x06B6252Fu, &in) == COS_OK && cos_rv_decode(&m_only, 0x02C58533u, &in) == COS_OK);
  /* width: RV64-only instructions do not exist in RV32 */
  static const uint8_t ONLY64[] = {COS_RV_LD, COS_RV_SD, COS_RV_ADDIW, COS_RV_SLLIW, COS_RV_MULW, COS_RV_AMOADD_D};
  for (size_t i = 0; i < COUNT(ONLY64); i++) {
    in.id = ONLY64[i];
    CK(s, cos_rv_encode(&RV32, &in, &w) == COS_E_NOT_FOUND);
    CK(s, cos_rv_encode(&RV64, &in, &w) == COS_OK);
  }
  CK(s, cos_rv_decode(&RV32, 0x00813503u, &in) == COS_E_INVALID && cos_rv_decode(&RV64, 0x00813503u, &in) == COS_OK); /* ld a0, 8(sp) */
  /* a nonsense profile is refused everywhere */
  in.id = COS_RV_ADD;
  CK(s, cos_rv_encode(&bad_width, &in, &w) == COS_E_INVALID && cos_rv_encode(&bad_exts, &in, &w) == COS_E_INVALID);
  CK(s, cos_rv_decode(&bad_width, 0x00C58533u, &in) == COS_E_INVALID && cos_rv_decode(&bad_exts, 0x00C58533u, &in) == COS_E_INVALID);
  CK(s, cos_rv_row_of(&bad_width, COS_RV_ADD) == NULL && cos_rv_row_by_name(&bad_exts, "add", 3) == NULL);
  CK(s, cos_rv_encode(NULL, &in, &w) == COS_E_INVALID && cos_rv_decode(NULL, 0x00C58533u, &in) == COS_E_INVALID);
  CK(s, cos_rv_encode(&RV32, NULL, &w) == COS_E_INVALID && cos_rv_encode(&RV32, &in, NULL) == COS_E_INVALID);
  CK(s, cos_rv_decode(&RV32, 0x00C58533u, NULL) == COS_E_INVALID);
  CK(s, !cos_rv_canonical(&RV32, NULL) && !cos_rv_canonical(NULL, &in));
  /* mnemonic lookup: exact length, per width */
  const CosRvRow *r = cos_rv_row_by_name(&RV32, "add", 3);
  CK(s, r != NULL && r->id == COS_RV_ADD);
  r = cos_rv_row_by_name(&RV32, "addi", 4);
  CK(s, r != NULL && r->id == COS_RV_ADDI);
  CK(s, cos_rv_row_by_name(&RV32, "addiw", 5) == NULL && cos_rv_row_by_name(&RV64, "addiw", 5) != NULL);
  CK(s, cos_rv_row_by_name(&RV32, "ad", 2) == NULL && cos_rv_row_by_name(&RV32, "", 0) == NULL && cos_rv_row_by_name(&RV32, NULL, 3) == NULL);
  CK(s, cos_rv_row_by_name(&RV32, "aaaaaaaaaaaa", 12) == NULL && cos_rv_row_by_name(&RV32, "nonsense", 8) == NULL);
}

/* ---------------------------------------------------------------- 5. sweeps */
static void make(const CosRvRow *r, unsigned v, CosRvInsn *in) {
  static const uint8_t RS[4][3] = {{5, 6, 7}, {31, 30, 29}, {0, 0, 0}, {1, 31, 2}};
  static const int32_t I12[4] = {-17, 2047, -2048, 0}, SH5[4] = {17, 31, 0, 1}, SH6[4] = {33, 63, 0, 32}, BR[4] = {16, 4094, -4096, -2}, UI[4] = {0x12345, 0xFFFFF, 0, 1},
                       JI[4] = {2048, 1048574, -1048576, -2}, CS[4] = {0x305, 4095, 0, 0xF14}, FE[4] = {0x0F3, 0x833, 0x0FF, 0x001};
  memset(in, 0, sizeof *in);
  in->id = r->id;
  const uint8_t rd = RS[v][0], rs1 = RS[v][1], rs2 = RS[v][2];
  switch ((CosRvCls)r->cls) {
    case COS_RV_CLS_R: in->rd = rd, in->rs1 = rs1, in->rs2 = rs2; break;
    case COS_RV_CLS_I:
    case COS_RV_CLS_L:
    case COS_RV_CLS_JR: in->rd = rd, in->rs1 = rs1, in->imm = I12[v]; break;
    case COS_RV_CLS_SH5: in->rd = rd, in->rs1 = rs1, in->imm = SH5[v]; break;
    case COS_RV_CLS_SH6: in->rd = rd, in->rs1 = rs1, in->imm = SH6[v]; break;
    case COS_RV_CLS_S: in->rs1 = rs1, in->rs2 = rs2, in->imm = I12[v]; break;
    case COS_RV_CLS_B: in->rs1 = rs1, in->rs2 = rs2, in->imm = BR[v]; break;
    case COS_RV_CLS_U: in->rd = rd, in->imm = UI[v]; break;
    case COS_RV_CLS_J: in->rd = rd, in->imm = JI[v]; break;
    case COS_RV_CLS_AMO: in->rd = rd, in->rs1 = rs1, in->rs2 = rs2, in->aq = (uint8_t)(v & 1u), in->rl = (uint8_t)((v >> 1) & 1u); break;
    case COS_RV_CLS_LR: in->rd = rd, in->rs1 = rs1, in->aq = (uint8_t)(v & 1u), in->rl = (uint8_t)((v >> 1) & 1u); break;
    case COS_RV_CLS_CSR:
    case COS_RV_CLS_CSRI: in->rd = rd, in->rs1 = rs1, in->imm = CS[v]; break;
    case COS_RV_CLS_FENCE: in->imm = FE[v]; break;
    case COS_RV_CLS_FENCEI:
    case COS_RV_CLS_N:
    case COS_RV_CLS_COUNT:
    default: break;
  }
}

static void test_sweeps(CosSuite *s) {
  const CosRvIsa isas[3] = {COS_RV_ISA_RV32, COS_RV_ISA_RV64, COS_RV_ISA_APP32};
  /* (a) every instruction of the profile, four operand sets: encode, decode, and the same fields come back; the mnemonic lookup finds the same row */
  for (unsigned k = 0; k < 3u; k++) {
    unsigned seen = 0, bad = 0;
    for (size_t i = 0; i < COS_RV_ROWS; i++) {
      const CosRvRow *r = &cos_rv_rows[i];
      if (cos_rv_row_of(&isas[k], r->id) != r) continue; /* not in this profile (or the other width's row of a shared id) */
      seen++;
      if (cos_rv_row_by_name(&isas[k], r->name, strlen(r->name)) != r) bad++;
      for (unsigned v = 0; v < 4u; v++) {
        CosRvInsn in, back;
        uint32_t w = 0;
        char text[64];
        make(r, v, &in);
        if (cos_rv_encode(&isas[k], &in, &w) != COS_OK || cos_rv_decode(&isas[k], w, &back) != COS_OK || !same(&in, &back) || (w & r->mask) != r->match ||
            cos_rv_format(&isas[k], &back, text, sizeof text) < strlen(r->name) || strncmp(text, r->name, strlen(r->name)) != 0)
          bad++;
      }
    }
    CK(s, bad == 0u);
    CK(s, seen > 40u); /* the profile is not empty: the loop above really ran */
  }
  /* (b) uniform words: whatever decodes encodes back to the same word; a refusal leaves the output untouched */
  for (unsigned k = 0; k < 3u; k++) {
    uint32_t x = 0x2545F491u + k;
    unsigned accepted = 0, bad = 0;
    for (unsigned n = 0; n < 400000u; n++) {
      const uint32_t w = xs(&x) | 3u;
      CosRvInsn in, sentinel;
      uint32_t again = 0;
      char text[64];
      memset(&sentinel, 0xEE, sizeof sentinel);
      in = sentinel;
      if (cos_rv_decode(&isas[k], w, &in) == COS_OK) {
        accepted++;
        const CosRvRow *r = cos_rv_row_of(&isas[k], in.id);
        if (r == NULL || (w & r->mask) != r->match || cos_rv_encode(&isas[k], &in, &again) != COS_OK || again != w ||
            cos_rv_format(&isas[k], &in, text, sizeof text) == 0u)
          bad++;
      } else if (memcmp(&in, &sentinel, sizeof in) != 0) {
        bad++;
      }
    }
    CK(s, bad == 0u);
    CK(s, accepted > 20000u); /* the sweep is not vacuous */
  }
  /* (c) words that are not 32-bit instructions: low bits other than 11, the reserved 48-bit-and-longer patterns, all zero, all ones */
  {
    uint32_t x = 0x9E3779B9u;
    unsigned bad = 0;
    for (unsigned n = 0; n < 100000u; n++) {
      const uint32_t w = xs(&x);
      CosRvInsn in;
      if ((w & 3u) != 3u && cos_rv_decode(&RV64, w, &in) != COS_E_INVALID) bad++;
      if ((w & 0x1Fu) == 0x1Fu && cos_rv_decode(&RV64, w, &in) != COS_E_INVALID) bad++; /* bits [4:0] = 11111: the 48-bit-or-longer format */
    }
    CK(s, bad == 0u);
    CosRvInsn in;
    CK(s, cos_rv_decode(&RV64, 0u, &in) == COS_E_INVALID && cos_rv_decode(&RV64, 0xFFFFFFFFu, &in) == COS_E_INVALID);
  }
}

/* ---------------------------------------------------------------- 6. text, names, flags, the table itself */
static void test_text(CosSuite *s) {
  CosRvInsn in = {COS_RV_ADDI, 10, 10, 0, 0, 0, 1};
  char buf[32];
  memset(buf, 'X', sizeof buf);
  CK(s, cos_rv_format(&RV32, &in, buf, 15u) == 14u && strcmp(buf, "addi a0, a0, 1") == 0); /* exact fit: 14 characters and the NUL */
  CK(s, cos_rv_format(&RV32, &in, buf, 14u) == 0u && buf[0] == '\0');                       /* one short: refused, still terminated */
  CK(s, cos_rv_format(&RV32, &in, buf, 1u) == 0u && buf[0] == '\0');
  memset(buf, 'X', sizeof buf);
  CK(s, cos_rv_format(&RV32, &in, buf, 0u) == 0u && buf[0] == 'X'); /* cap 0: not touched */
  CK(s, cos_rv_format(&RV32, &in, NULL, 8u) == 0u && cos_rv_format(&RV32, NULL, buf, 8u) == 0u && buf[0] == '\0');
  in.id = COS_RV_LD;
  CK(s, cos_rv_format(&RV32, &in, buf, sizeof buf) == 0u && buf[0] == '\0'); /* ld is no RV32 instruction */
  CK(s, cos_rv_format(&RV64, &in, buf, sizeof buf) == strlen("ld a0, 1(a0)") && strcmp(buf, "ld a0, 1(a0)") == 0);
  /* register names */
  CK(s, strcmp(cos_rv_reg_name(0), "zero") == 0 && strcmp(cos_rv_reg_name(1), "ra") == 0 && strcmp(cos_rv_reg_name(2), "sp") == 0 && strcmp(cos_rv_reg_name(3), "gp") == 0 &&
         strcmp(cos_rv_reg_name(4), "tp") == 0 && strcmp(cos_rv_reg_name(10), "a0") == 0 && strcmp(cos_rv_reg_name(17), "a7") == 0 && strcmp(cos_rv_reg_name(8), "s0") == 0 &&
         strcmp(cos_rv_reg_name(31), "t6") == 0 && strcmp(cos_rv_reg_name(32), "?") == 0);
  unsigned distinct = 0;
  for (unsigned a = 0; a < 32u; a++) {
    unsigned same_as = 0;
    for (unsigned b = 0; b < a; b++) same_as += strcmp(cos_rv_reg_name(a), cos_rv_reg_name(b)) == 0 ? 1u : 0u;
    distinct += same_as == 0u && cos_rv_reg_name(a)[0] != '\0' ? 1u : 0u;
  }
  CK(s, distinct == 32u);
  /* CSR names depend on the register width: the odd pmpcfg registers and the *h upper halves exist in RV32 only (riscv-privileged 3.7.1) */
  CK(s, strcmp(cos_rv_csr_name(32u, 0x300u), "mstatus") == 0 && strcmp(cos_rv_csr_name(64u, 0x300u), "mstatus") == 0);
  CK(s, strcmp(cos_rv_csr_name(32u, 0xF14u), "mhartid") == 0 && strcmp(cos_rv_csr_name(64u, 0xF14u), "mhartid") == 0);
  CK(s, strcmp(cos_rv_csr_name(32u, 0x3A1u), "pmpcfg1") == 0 && cos_rv_csr_name(64u, 0x3A1u) == NULL);
  CK(s, strcmp(cos_rv_csr_name(32u, 0x3A2u), "pmpcfg2") == 0 && strcmp(cos_rv_csr_name(64u, 0x3A2u), "pmpcfg2") == 0);
  CK(s, strcmp(cos_rv_csr_name(32u, 0x310u), "mstatush") == 0 && cos_rv_csr_name(64u, 0x310u) == NULL);
  CK(s, cos_rv_csr_name(16u, 0x300u) == NULL && cos_rv_csr_name(32u, 0x7FFu) == NULL);
  /* a CSR number with no name in this width is printed as a number, and the instruction itself stays valid */
  CosRvInsn csr = {COS_RV_CSRRW, 10, 11, 0, 0, 0, 0x3A1};
  CK(s, cos_rv_format(&RV32, &csr, buf, sizeof buf) > 0u && strcmp(buf, "csrrw a0, pmpcfg1, a1") == 0);
  CK(s, cos_rv_format(&RV64, &csr, buf, sizeof buf) > 0u && strcmp(buf, "csrrw a0, 0x3a1, a1") == 0);
  /* reserved fields: decoded (the specification says to execute them) but flagged, so that the verifier can refuse them */
  CosRvInsn d;
  CK(s, cos_rv_decode(&RV32, 0x0330008Fu, &d) == COS_OK && !cos_rv_canonical(&RV32, &d)); /* fence with rd = 1 */
  CK(s, cos_rv_decode(&RV32, 0x0331800Fu, &d) == COS_OK && !cos_rv_canonical(&RV32, &d)); /* fence with rs1 = 1 */
  CK(s, cos_rv_decode(&RV32, 0x1330000Fu, &d) == COS_OK && !cos_rv_canonical(&RV32, &d)); /* fence with fm = 1 */
  CK(s, cos_rv_decode(&RV32, 0x0010100Fu, &d) == COS_OK && !cos_rv_canonical(&RV32, &d)); /* fence.i with a set reserved field */
  CK(s, cos_rv_decode(&RV32, 0x8330000Fu, &d) == COS_OK && cos_rv_canonical(&RV32, &d));  /* fence.tso */
  CK(s, cos_rv_decode(&RV32, 0x0330000Fu, &d) == COS_OK && cos_rv_canonical(&RV32, &d));  /* fence rw, rw */
  CK(s, cos_rv_decode(&RV32, 0x0000100Fu, &d) == COS_OK && cos_rv_canonical(&RV32, &d));  /* fence.i */
}

static void test_table(CosSuite *s) {
  unsigned bad = 0;
  for (size_t i = 0; i < COS_RV_ROWS; i++) {
    const CosRvRow *a = &cos_rv_rows[i];
    if (a->name[0] == '\0' || memchr(a->name, '\0', sizeof a->name) == NULL) bad++;       /* a name, terminated inside its field */
    if ((a->match & ~a->mask) != 0u) bad++;                                                /* the match lies inside the mask */
    if ((a->mask & 3u) != 3u || (a->match & 3u) != 3u) bad++;                              /* 32-bit instructions only */
    if (a->id >= COS_RV_OP_COUNT || a->cls >= COS_RV_CLS_COUNT) bad++;
    if (a->xlens < 1u || a->xlens > 3u || (a->ext & ~COS_RV_EXT_ALL) != 0u) bad++;
    for (size_t j = i + 1u; j < COS_RV_ROWS; j++) {
      const CosRvRow *b = &cos_rv_rows[j];
      if ((a->xlens & b->xlens) != 0u && (a->mask & b->mask & (a->match ^ b->match)) == 0u) bad++; /* two rows that one word could match */
    }
  }
  CK(s, bad == 0u);
  unsigned n32 = 0, n64 = 0, napp32 = 0;
  for (size_t i = 0; i < COS_RV_ROWS; i++) {
    n32 += (cos_rv_rows[i].xlens & 1u) != 0u ? 1u : 0u;
    n64 += (cos_rv_rows[i].xlens & 2u) != 0u ? 1u : 0u;
    napp32 += (cos_rv_rows[i].xlens & 1u) != 0u && (cos_rv_rows[i].ext & ~(COS_RV_EXT_M | COS_RV_EXT_A)) == 0u ? 1u : 0u;
  }
  CK(s, n32 > 60u && n64 > n32 - 5u && napp32 > 50u && napp32 < n32); /* the application profile is a strict subset of the machine profile */
  bad = 0;
  for (size_t i = 0; i < COS_RV_CSRS; i++) {
    const CosRvCsrName *c = &cos_rv_csr_names[i];
    if (c->name[0] == '\0' || memchr(c->name, '\0', sizeof c->name) == NULL || c->addr > 4095u || c->xlens < 1u || c->xlens > 3u) bad++;
    if (i > 0u && cos_rv_csr_names[i - 1u].addr >= c->addr) bad++; /* sorted, no duplicate address */
    for (size_t j = 0; j < i; j++) {
      if (strcmp(cos_rv_csr_names[j].name, c->name) == 0) bad++; /* no duplicate name */
    }
  }
  CK(s, bad == 0u);
}

int main(void) {
  CosSuite s = {"rv", 0, 0};
  test_vectors(&s);
  test_encode(&s);
  test_edges(&s);
  test_profiles(&s);
  test_sweeps(&s);
  test_text(&s);
  test_table(&s);
  return suite_done(&s);
}
