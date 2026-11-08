/* Host helper for tools/check_rv.py: drives cos_rv.c from text on stdin so that an independent oracle (llvm-mc, llvm-objdump) can be compared with it.
 *   c17_rv_dump rows  XLEN EXTS            : one line per table row:  name class match mask xlens ext
 *   c17_rv_dump enc   XLEN EXTS  < lines   : "name rd rs1 rs2 aq rl imm"  ->  "OK 0x%08x"  or  "ERR <status>"
 *   c17_rv_dump dec   XLEN EXTS  < lines   : "0xWORD"                      ->  "OK <canonical 0|1> <text>"  or  "ILLEGAL"
 * EXTS is a number (bit mask of COS_RV_EXT_*). This is a test tool, not part of the kernel: it uses stdio. */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cos_rv.h"

static const char *const CLS[] = {"R", "I", "L", "JR", "SH5", "SH6", "S", "B", "U", "J", "AMO", "LR", "CSR", "CSRI", "FENCE", "FENCEI", "N"};

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: c17_rv_dump rows|enc|dec XLEN EXTS\n");
    return 2;
  }
  const CosRvIsa isa = {(uint8_t)atoi(argv[2]), (uint8_t)atoi(argv[3])};
  char line[512];
  if (strcmp(argv[1], "rows") == 0) { /* the whole table: name class match mask xlens ext (the profile arguments are ignored) */
    for (unsigned i = 0; i < COS_RV_ROWS; i++) {
      const CosRvRow *r = &cos_rv_rows[i];
      printf("%s %s 0x%08" PRIx32 " 0x%08" PRIx32 " %u %u\n", r->name, CLS[r->cls], r->match, r->mask, (unsigned)r->xlens, (unsigned)r->ext);
    }
    return 0;
  }
  if (strcmp(argv[1], "enc") == 0) {
    while (fgets(line, sizeof line, stdin) != NULL) {
      char name[32];
      int rd, rs1, rs2, aq, rl, imm;
      if (sscanf(line, "%31s %d %d %d %d %d %d", name, &rd, &rs1, &rs2, &aq, &rl, &imm) != 7) {
        puts("BADLINE");
        continue;
      }
      const CosRvRow *r = cos_rv_row_by_name(&isa, name, strlen(name));
      if (r == NULL) {
        puts("ERR not_found");
        continue;
      }
      CosRvInsn in = {r->id, (uint8_t)rd, (uint8_t)rs1, (uint8_t)rs2, (uint8_t)aq, (uint8_t)rl, imm};
      uint32_t w = 0;
      const CosStatus st = cos_rv_encode(&isa, &in, &w);
      if (st == COS_OK) printf("OK 0x%08" PRIx32 "\n", w);
      else printf("ERR %s\n", cos_status_name(st));
    }
    return 0;
  }
  if (strcmp(argv[1], "dec") == 0) {
    while (fgets(line, sizeof line, stdin) != NULL) {
      const uint32_t w = (uint32_t)strtoul(line, NULL, 0);
      CosRvInsn in;
      char text[96];
      if (cos_rv_decode(&isa, w, &in) != COS_OK) {
        puts("ILLEGAL");
        continue;
      }
      /* the decoder must give back exactly the word it was given (every field is carried) */
      uint32_t again = 0;
      if (cos_rv_encode(&isa, &in, &again) != COS_OK || again != w) {
        printf("ROUNDTRIP-FAIL 0x%08" PRIx32 " 0x%08" PRIx32 "\n", w, again);
        continue;
      }
      if (cos_rv_format(&isa, &in, text, sizeof text) == 0u) {
        puts("FORMAT-FAIL");
        continue;
      }
      printf("OK %d %s\n", cos_rv_canonical(&isa, &in) ? 1 : 0, text);
    }
    return 0;
  }
  return 2;
}
