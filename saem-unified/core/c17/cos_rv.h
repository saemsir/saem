/* QCOS assembly layer, part 1: the RISC-V instruction word as a pure function.
 *
 *   encode  : operands -> 32-bit word   (every field range-checked; a bad operand is refused, never truncated)
 *   decode  : 32-bit word -> operands   (strict: a word that is not an instruction of the profile is refused)
 *   format  : operands -> canonical assembly text (no aliases: what the hardware decodes, nothing more)
 *
 * Rules (same as the rest of core/c17): C17, zero heap, no globals (the tables are const), no libc but memcpy-class calls, no floating point.
 * The opcode TABLE is generated (tools/gen_rv_tab.py from riscv-opcodes); the operand FORMATS are here and are proven against
 * llvm-mc / llvm-objdump by tools/check_rv.py, so no opcode and no format is taken on trust.
 *
 * Register width and extensions are parameters, not compile-time switches: one binary encodes and decodes RV32 and RV64.
 * The profile (CosRvIsa.exts) is also a safety tool: a word that decodes only under an extension that is switched off is ILLEGAL,
 * so the monitor core can verify an application image against the application profile (no CSR, no mret, no wfi).
 *
 * Compressed (16-bit) instructions are NOT handled here: the emitter writes 32-bit words only (project instruction 2026-11-01),
 * and a word whose two low bits are not 11 is refused. [P] decode-only support for the C extension, needed to run compiler output.
 *
 * Immediate conventions (the value in CosRvInsn.imm):
 *   I, L, JR, S : signed 12 bit           B : signed 13 bit, even (byte offset from this instruction)     J : signed 21 bit, even
 *   U           : the 20-bit FIELD value 0..0xFFFFF (the instruction adds imm << 12)
 *   SH5 / SH6   : shift amount 0..31 / 0..63 (SH6 exists in RV64 only; in RV32 the shift immediates are SH5)
 *   CSR         : csr number 0..4095 (rs1 is the source register);  CSRI : csr number in imm, the 5-bit immediate in rs1
 *   FENCE       : imm = fm << 8 | pred << 4 | succ (as in the encoding); rs1 and rd are the reserved fields (zero when canonical)
 *   AMO / LR    : aq and rl are 0 or 1
 * Fields a class does not use are ignored by encode and set to 0 by decode. */
#ifndef COS_RV_H
#define COS_RV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cos_rv_ops.h"
#include "cos_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CosRvRow {
  uint32_t match;  /* (word & mask) == match selects this instruction */
  uint32_t mask;
  char name[12];
  uint8_t id;      /* CosRvOp */
  uint8_t cls;     /* CosRvCls */
  uint8_t xlens;   /* bit 0: exists in RV32, bit 1: exists in RV64 */
  uint8_t ext;     /* COS_RV_EXT_* bit, 0 = base integer ISA */
} CosRvRow;

typedef struct CosRvCsrName {
  uint16_t addr;
  uint8_t xlens;   /* bit 0: the register exists in RV32, bit 1: in RV64 (pmpcfg1, pmpcfg3, ... and the *h halves are RV32 only) */
  char name[16];
} CosRvCsrName;

extern const CosRvRow cos_rv_rows[COS_RV_ROWS];
extern const CosRvCsrName cos_rv_csr_names[COS_RV_CSRS];

typedef struct CosRvIsa {
  uint8_t xlen;  /* 32 or 64 */
  uint8_t exts;  /* COS_RV_EXT_* bits that are switched on; the base integer ISA is always on */
} CosRvIsa;

typedef struct CosRvInsn {
  uint8_t id;   /* CosRvOp */
  uint8_t rd, rs1, rs2;
  uint8_t aq, rl;
  int32_t imm;
} CosRvInsn;

/* The profiles we use. */
#define COS_RV_ISA_RV32 {32u, COS_RV_EXT_ALL}   /* machine-mode profile: everything this layer knows */
#define COS_RV_ISA_RV64 {64u, COS_RV_EXT_ALL}
#define COS_RV_ISA_APP32 {32u, COS_RV_EXT_M | COS_RV_EXT_A}  /* application profile: no CSR, no fence.i, no mret, no wfi */
#define COS_RV_ISA_APP64 {64u, COS_RV_EXT_M | COS_RV_EXT_A}

/* The row of an instruction in this profile, or NULL (unknown id, wrong width, extension off). */
const CosRvRow *cos_rv_row_of(const CosRvIsa *isa, uint8_t id);
/* The row for a mnemonic of `len` characters, or NULL. */
const CosRvRow *cos_rv_row_by_name(const CosRvIsa *isa, const char *name, size_t len);

/* COS_OK; COS_E_INVALID (bad isa, register above 31, aq/rl above 1); COS_E_NOT_FOUND (the profile has no such instruction);
 * COS_E_RANGE (immediate out of range or odd where it must be even). *word is written only on COS_OK. */
CosStatus cos_rv_encode(const CosRvIsa *isa, const CosRvInsn *in, uint32_t *word);
/* COS_OK, or COS_E_INVALID: not a 32-bit instruction of this profile. *out is written only on COS_OK. */
CosStatus cos_rv_decode(const CosRvIsa *isa, uint32_t word, CosRvInsn *out);
/* true if the reserved fields are zero and a fence is one of the two forms the specification defines (fence with fm 0, fence.tso). */
bool cos_rv_canonical(const CosRvIsa *isa, const CosRvInsn *in);

/* Canonical text into buf (always NUL-terminated when cap > 0). Returns the length without the NUL; 0 if the instruction is not in the profile or buf is too small.
 * Branch and jump targets are written as ".+N" / ".-N" (relative to this instruction), which llvm-mc reads back. */
size_t cos_rv_format(const CosRvIsa *isa, const CosRvInsn *in, char *buf, size_t cap);

const char *cos_rv_reg_name(unsigned r);        /* "zero", "ra", "sp", ... "t6"; "?" above 31 */
const char *cos_rv_csr_name(unsigned xlen, unsigned addr);  /* "mstatus", ...; NULL if that register has no name in this width (xlen 32 or 64) */

#ifdef __cplusplus
}
#endif
#endif
