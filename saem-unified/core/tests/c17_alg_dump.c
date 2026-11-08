/* Helper for tools/check_alg.py: prints what the C tower (cos_alg.c) computes, so Python can compare it with the independent oracle tools/alg_oracle.py.
 * Reads one command per line from stdin and prints one answer line per command. An element is always 8 tokens "n/d" (its components, the unused ones
 * included, so a nonzero unused component reaches the canonical-form check); operands are raw integers, never made by cos_rat_make, so malformed
 * rationals reach the validity checks as well. Every algebra result starts as 12345/678 in all 8 slots, so a refusal shows the slots were left alone.
 *     add|sub|mul L A B            ->  "status c0 ... c7"          (each c is "n/d")
 *     neg|conj|inv L A             ->  "status c0 ... c7"
 *     scale L n/d A                ->  "status c0 ... c7"
 *     embed FROM TO A              ->  "status c0 ... c7"
 *     norm2 L A                    ->  "status n/d"                (starts as 12345/678)
 *     lmat|rmat L A ROW COL        ->  "status n/d"
 *     sign L I J                   ->  the integer cos_alg_basis_sign(L, I, J)
 *     dim L                        ->  the integer cos_alg_dim(L)
 *     valid L A | zero L A         ->  1 or 0
 *     eq L A B                     ->  1 or 0
 * Test tooling only: it uses stdio, the kernel itself does not. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cos_alg.h"

#define SENT_N 12345
#define SENT_D 678

static const char *rd_rat(const char *s, CosRat *r) {
  long long n = 0, d = 0;
  int used = 0;
  if (sscanf(s, " %lld/%lld%n", &n, &d, &used) != 2) return NULL;
  r->num = (int64_t)n;
  r->den = (int64_t)d;
  return s + used;
}

static const char *rd_alg(const char *s, CosAlg *a) {
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM && s != NULL; ++k) s = rd_rat(s, &a->c[k]);
  return s;
}

static const char *rd_uint(const char *s, uint32_t *v) {
  unsigned long long x = 0;
  int used = 0;
  if (sscanf(s, " %llu%n", &x, &used) != 1) return NULL;
  *v = (uint32_t)x;
  return s + used;
}

static void sentinel(CosAlg *a) {
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) {
    a->c[k].num = SENT_N;
    a->c[k].den = SENT_D;
  }
}

static void print_alg(CosStatus st, const CosAlg *a) {
  printf("%d", (int)st);
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) printf(" %lld/%lld", (long long)a->c[k].num, (long long)a->c[k].den);
  putchar('\n');
}

int main(void) {
  static char line[4096];
  while (fgets(line, sizeof line, stdin) != NULL) {
    char op[16] = {0};
    int used = 0;
    if (sscanf(line, "%15s%n", op, &used) != 1) return 2;
    const char *p = line + used;
    uint32_t L = 0;
    CosAlg a, b, out;
    CosRat r = {SENT_N, SENT_D}, sc = {0, 1};
    sentinel(&out);
    if (strcmp(op, "embed") == 0) {
      uint32_t to = 0;
      if ((p = rd_uint(p, &L)) == NULL || (p = rd_uint(p, &to)) == NULL || (p = rd_alg(p, &a)) == NULL) return 2;
      print_alg(cos_alg_embed(L, to, &a, &out), &out);
      continue;
    }
    if ((p = rd_uint(p, &L)) == NULL) return 2;
    if (strcmp(op, "sign") == 0) {
      uint32_t i = 0, j = 0;
      if ((p = rd_uint(p, &i)) == NULL || (p = rd_uint(p, &j)) == NULL) return 2;
      printf("%d\n", cos_alg_basis_sign(L, i, j));
    } else if (strcmp(op, "dim") == 0) {
      printf("%u\n", (unsigned)cos_alg_dim(L));
    } else if (strcmp(op, "add") == 0 || strcmp(op, "sub") == 0 || strcmp(op, "mul") == 0) {
      if ((p = rd_alg(p, &a)) == NULL || (p = rd_alg(p, &b)) == NULL) return 2;
      const CosStatus st = op[0] == 'a' ? cos_alg_add(L, &a, &b, &out) : op[0] == 's' ? cos_alg_sub(L, &a, &b, &out) : cos_alg_mul(L, &a, &b, &out);
      print_alg(st, &out);
    } else if (strcmp(op, "neg") == 0 || strcmp(op, "conj") == 0 || strcmp(op, "inv") == 0) {
      if ((p = rd_alg(p, &a)) == NULL) return 2;
      const CosStatus st = op[0] == 'n' ? cos_alg_neg(L, &a, &out) : op[0] == 'c' ? cos_alg_conj(L, &a, &out) : cos_alg_inv(L, &a, &out);
      print_alg(st, &out);
    } else if (strcmp(op, "scale") == 0) {
      if ((p = rd_rat(p, &sc)) == NULL || (p = rd_alg(p, &a)) == NULL) return 2;
      print_alg(cos_alg_scale(L, sc, &a, &out), &out);
    } else if (strcmp(op, "norm2") == 0) {
      if ((p = rd_alg(p, &a)) == NULL) return 2;
      const CosStatus st = cos_alg_norm2(L, &a, &r);
      printf("%d %lld/%lld\n", (int)st, (long long)r.num, (long long)r.den);
    } else if (strcmp(op, "lmat") == 0 || strcmp(op, "rmat") == 0) {
      uint32_t row = 0, col = 0;
      if ((p = rd_alg(p, &a)) == NULL || (p = rd_uint(p, &row)) == NULL || (p = rd_uint(p, &col)) == NULL) return 2;
      const CosStatus st = op[0] == 'l' ? cos_alg_lmat_entry(L, &a, row, col, &r) : cos_alg_rmat_entry(L, &a, row, col, &r);
      printf("%d %lld/%lld\n", (int)st, (long long)r.num, (long long)r.den);
    } else if (strcmp(op, "valid") == 0 || strcmp(op, "zero") == 0) {
      if ((p = rd_alg(p, &a)) == NULL) return 2;
      printf("%d\n", (op[0] == 'v' ? cos_alg_valid(L, &a) : cos_alg_is_zero(L, &a)) ? 1 : 0);
    } else if (strcmp(op, "eq") == 0) {
      if ((p = rd_alg(p, &a)) == NULL || (p = rd_alg(p, &b)) == NULL) return 2;
      printf("%d\n", cos_alg_eq(L, &a, &b) ? 1 : 0);
    } else {
      return 2;
    }
  }
  return 0;
}
