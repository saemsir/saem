/* The number tower of the Work-Time Engine (cos_alg): every line of the integrity contract in cos_alg.h as an exact test, at levels 0 (Q), 1 (Q(i)),
 * 2 (Hamilton's quaternions) and 3 (octonions). Nothing here is approximate: components are exact rationals, so an identity either holds or it does not.
 * The broad comparison with an independent implementation (Python Fractions, the pair rule, thousands of random operands, three classes of operand size,
 * the sign table, Baez's convention, the sedenion zero divisors) is in tools/check_alg.py.
 *
 * Where a theorem comes from (see the header for the full list): Baez, "The Octonions", sections 1.1, 2.2, 2.3, 4.1; Artin, "Algebra", 9.3 and 9.4;
 * MIT 18.755 section 6.2. A "witness" is a concrete counterexample that shows a law FAILS at a level, so the tests pin down where each law stops. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "c17_check.h"
#include "cos_alg.h"

#define MAXV INT64_MAX
#define NLEV 4u /* levels 0..3 */

/* ---------------------------------------------------------------------------------------------------------------------------------- helpers */
typedef struct Rng {
  uint64_t s;
} Rng;

static uint64_t rnext(Rng *r) { /* xorshift64*: deterministic, no global state */
  uint64_t x = r->s;
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  r->s = x;
  return x * 0x2545F4914F6CDD1Dull;
}
static int rr(Rng *r, int lo, int hi) { return lo + (int)(rnext(r) % (uint64_t)(hi - lo + 1)); }

static CosRat Q(int64_t n, int64_t d) {
  CosRat r = {0, 1};
  (void)cos_rat_make(n, d, &r);
  return r;
}
static bool qeq(CosRat a, CosRat b) { return a.num == b.num && a.den == b.den; }

static CosAlg sentinel(void) { /* a valid value no operation produces by accident: it proves an error leaves *out alone */
  CosAlg a;
  cos_alg_zero(&a);
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) a.c[k] = Q(7, 9);
  return a;
}
static bool same(const CosAlg *a, const CosAlg *b) { return memcmp(a, b, sizeof *a) == 0; }

static CosAlg rnd(Rng *g, uint32_t L) { /* components n/d with |n| <= 6 and d in 1..4: every product and sum of the identities below fits 64 bits */
  CosAlg a;
  cos_alg_zero(&a);
  for (uint32_t k = 0; k < cos_alg_dim(L); ++k) a.c[k] = Q(rr(g, -6, 6), rr(g, 1, 4));
  return a;
}
static CosAlg rnd_nz(Rng *g, uint32_t L) {
  CosAlg a = rnd(g, L);
  if (cos_alg_is_zero(L, &a)) a.c[0] = Q(1, 1);
  return a;
}
static CosAlg rnd_im(Rng *g, uint32_t L) { /* an imaginary element: real part 0 */
  CosAlg a = rnd(g, L);
  a.c[0] = Q(0, 1);
  return a;
}
static CosRat rnd_s(Rng *g) { return Q(rr(g, -5, 5), rr(g, 1, 4)); }

static CosAlg E(uint32_t L, uint32_t i) {
  CosAlg a;
  cos_alg_zero(&a);
  (void)cos_alg_basis(L, i, &a);
  return a;
}
static CosAlg real(CosRat r) {
  CosAlg a;
  cos_alg_zero(&a);
  a.c[0] = r;
  return a;
}
static bool is_real(uint32_t L, CosAlg a) {
  for (uint32_t k = 1; k < cos_alg_dim(L); ++k)
    if (a.c[k].num != 0) return false;
  return true;
}
static bool eq(uint32_t L, CosAlg a, CosAlg b) { return cos_alg_eq(L, &a, &b); }
static bool zero(uint32_t L, CosAlg a) { return cos_alg_is_zero(L, &a); }

/* every wrapper checks the status, so a refusal inside a test is itself a failure */
static CosAlg mu(CosSuite *s, uint32_t L, CosAlg a, CosAlg b) {
  CosAlg r = sentinel();
  CK(s, cos_alg_mul(L, &a, &b, &r) == COS_OK);
  return r;
}
static CosAlg ad(CosSuite *s, uint32_t L, CosAlg a, CosAlg b) {
  CosAlg r = sentinel();
  CK(s, cos_alg_add(L, &a, &b, &r) == COS_OK);
  return r;
}
static CosAlg sb(CosSuite *s, uint32_t L, CosAlg a, CosAlg b) {
  CosAlg r = sentinel();
  CK(s, cos_alg_sub(L, &a, &b, &r) == COS_OK);
  return r;
}
static CosAlg ng(CosSuite *s, uint32_t L, CosAlg a) {
  CosAlg r = sentinel();
  CK(s, cos_alg_neg(L, &a, &r) == COS_OK);
  return r;
}
static CosAlg cj(CosSuite *s, uint32_t L, CosAlg a) {
  CosAlg r = sentinel();
  CK(s, cos_alg_conj(L, &a, &r) == COS_OK);
  return r;
}
static CosAlg iv(CosSuite *s, uint32_t L, CosAlg a) {
  CosAlg r = sentinel();
  CK(s, cos_alg_inv(L, &a, &r) == COS_OK);
  return r;
}
static CosAlg sc(CosSuite *s, uint32_t L, CosRat k, CosAlg a) {
  CosAlg r = sentinel();
  CK(s, cos_alg_scale(L, k, &a, &r) == COS_OK);
  return r;
}
static CosRat nm(CosSuite *s, uint32_t L, CosAlg a) {
  CosRat r = {123, 7};
  CK(s, cos_alg_norm2(L, &a, &r) == COS_OK);
  return r;
}
static CosRat qmul(CosSuite *s, CosRat a, CosRat b) {
  CosRat r = {0, 1};
  CK(s, cos_rat_mul(a, b, &r) == COS_OK);
  return r;
}
static CosRat qadd(CosSuite *s, CosRat a, CosRat b) {
  CosRat r = {0, 1};
  CK(s, cos_rat_add(a, b, &r) == COS_OK);
  return r;
}
static CosRat qsub(CosSuite *s, CosRat a, CosRat b) {
  CosRat r = {0, 1};
  CK(s, cos_rat_sub(a, b, &r) == COS_OK);
  return r;
}
static CosRat dot(CosSuite *s, uint32_t L, CosAlg a, CosAlg b) { /* the sum of products of components: the inner product */
  CosRat acc = {0, 1};
  for (uint32_t k = 0; k < cos_alg_dim(L); ++k) acc = qadd(s, acc, qmul(s, a.c[k], b.c[k]));
  return acc;
}
static CosAlg assoc(CosSuite *s, uint32_t L, CosAlg a, CosAlg b, CosAlg c) { /* the associator (ab)c - a(bc) */
  return sb(s, L, mu(s, L, mu(s, L, a, b), c), mu(s, L, a, mu(s, L, b, c)));
}
static CosAlg from(uint32_t L, const int64_t *v, size_t n) {
  CosAlg a = sentinel();
  (void)cos_alg_from_ints(L, v, n, &a);
  return a;
}

/* square matrices over Q up to 8 x 8: the regular representations */
typedef struct Mat {
  CosRat m[COS_ALG_MAX_DIM][COS_ALG_MAX_DIM];
} Mat;

static Mat mzero(void) {
  Mat M;
  for (uint32_t r = 0; r < COS_ALG_MAX_DIM; ++r)
    for (uint32_t c = 0; c < COS_ALG_MAX_DIM; ++c) M.m[r][c] = Q(0, 1);
  return M;
}
static Mat mscalar(uint32_t d, CosRat v) {
  Mat M = mzero();
  for (uint32_t k = 0; k < d; ++k) M.m[k][k] = v;
  return M;
}
static Mat lmat(CosSuite *s, uint32_t L, CosAlg a) {
  Mat M = mzero();
  for (uint32_t r = 0; r < cos_alg_dim(L); ++r)
    for (uint32_t c = 0; c < cos_alg_dim(L); ++c) CK(s, cos_alg_lmat_entry(L, &a, r, c, &M.m[r][c]) == COS_OK);
  return M;
}
static Mat rmat(CosSuite *s, uint32_t L, CosAlg b) {
  Mat M = mzero();
  for (uint32_t r = 0; r < cos_alg_dim(L); ++r)
    for (uint32_t c = 0; c < cos_alg_dim(L); ++c) CK(s, cos_alg_rmat_entry(L, &b, r, c, &M.m[r][c]) == COS_OK);
  return M;
}
static Mat mmul(CosSuite *s, uint32_t d, const Mat *A, const Mat *B) {
  Mat C = mzero();
  for (uint32_t r = 0; r < d; ++r)
    for (uint32_t c = 0; c < d; ++c) {
      CosRat acc = {0, 1};
      for (uint32_t k = 0; k < d; ++k) acc = qadd(s, acc, qmul(s, A->m[r][k], B->m[k][c]));
      C.m[r][c] = acc;
    }
  return C;
}
static Mat mtr(uint32_t d, const Mat *A) {
  Mat T = mzero();
  for (uint32_t r = 0; r < d; ++r)
    for (uint32_t c = 0; c < d; ++c) T.m[c][r] = A->m[r][c];
  return T;
}
static Mat mneg(CosSuite *s, uint32_t d, const Mat *A) {
  Mat N = mzero();
  for (uint32_t r = 0; r < d; ++r)
    for (uint32_t c = 0; c < d; ++c) N.m[r][c] = qsub(s, Q(0, 1), A->m[r][c]);
  return N;
}
static Mat madd(CosSuite *s, uint32_t d, const Mat *A, const Mat *B) {
  Mat S = mzero();
  for (uint32_t r = 0; r < d; ++r)
    for (uint32_t c = 0; c < d; ++c) S.m[r][c] = qadd(s, A->m[r][c], B->m[r][c]);
  return S;
}
static bool meq(uint32_t d, const Mat *A, const Mat *B) {
  for (uint32_t r = 0; r < d; ++r)
    for (uint32_t c = 0; c < d; ++c)
      if (!qeq(A->m[r][c], B->m[r][c])) return false;
  return true;
}
static CosAlg mvec(CosSuite *s, uint32_t L, const Mat *M, CosAlg x) {
  CosAlg y;
  cos_alg_zero(&y);
  for (uint32_t r = 0; r < cos_alg_dim(L); ++r) {
    CosRat acc = {0, 1};
    for (uint32_t c = 0; c < cos_alg_dim(L); ++c) acc = qadd(s, acc, qmul(s, M->m[r][c], x.c[c]));
    y.c[r] = acc;
  }
  return y;
}

/* ---------------------------------------------------------------------------------------------------------- the basis, the sign rule, the Fano plane */
static void test_signs(CosSuite *s) {
  CK(s, cos_alg_dim(0) == 1u && cos_alg_dim(1) == 2u && cos_alg_dim(2) == 4u && cos_alg_dim(3) == 8u);
  CK(s, cos_alg_dim(4) == 0u && cos_alg_dim(0xFFFFFFFFu) == 0u);
  for (uint32_t L = 0; L < NLEV; ++L) {
    const uint32_t d = cos_alg_dim(L);
    for (uint32_t i = 0; i < d; ++i)
      for (uint32_t j = 0; j < d; ++j) {
        const int sg = cos_alg_basis_sign(L, i, j);
        CK(s, sg == 1 || sg == -1);
        CosAlg want = E(L, i ^ j);
        if (sg < 0) want = ng(s, L, want);
        CK(s, eq(L, mu(s, L, E(L, i), E(L, j)), want)); /* the sign table is the product */
        if (i == 0 || j == 0) CK(s, sg == 1);              /* e_0 is the unit */
        if (i == j && i > 0) CK(s, sg == -1);              /* every imaginary unit squares to -1 */
        if (i != j && i > 0 && j > 0) CK(s, sg == -cos_alg_basis_sign(L, j, i)); /* distinct imaginary units anticommute */
      }
  }
  CK(s, cos_alg_basis_sign(4, 0, 0) == 0 && cos_alg_basis_sign(3, 8, 0) == 0 && cos_alg_basis_sign(3, 0, 8) == 0);
  CK(s, cos_alg_basis_sign(0, 1, 0) == 0 && cos_alg_basis_sign(1, 2, 0) == 0);
  /* Hamilton: i j = k, j k = i, k i = j, j i = -k, ijk = -1 (e1 e2 e3 at level 2) */
  CK(s, cos_alg_basis_sign(2, 1, 2) == 1 && cos_alg_basis_sign(2, 2, 3) == 1 && cos_alg_basis_sign(2, 3, 1) == 1);
  CK(s, cos_alg_basis_sign(2, 2, 1) == -1 && cos_alg_basis_sign(2, 3, 2) == -1 && cos_alg_basis_sign(2, 1, 3) == -1);
  CK(s, eq(2, mu(s, 2, mu(s, 2, E(2, 1), E(2, 2)), E(2, 3)), ng(s, 2, E(2, 0))));
  /* the Fano plane (Baez 2.1): the 7 triples {i, j, i xor j} are the lines; each is a quaternionic triple, so its products are cyclic */
  uint32_t lines = 0;
  for (uint32_t i = 1; i < 8; ++i)
    for (uint32_t j = 1; j < 8; ++j) {
      if (i == j) continue;
      const uint32_t k = i ^ j;
      CK(s, cos_alg_basis_sign(3, i, j) == cos_alg_basis_sign(3, j, k) && cos_alg_basis_sign(3, j, k) == cos_alg_basis_sign(3, k, i));
      if (i < j && k > j) ++lines;
    }
  CK(s, lines == 7u);
}

/* ------------------------------------------------------------------------------------------------------------------------------ the ring axioms */
static void test_ring(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    CosAlg z;
    cos_alg_zero(&z);
    const CosAlg one = E(L, 0);
    for (int n = 0; n < 40; ++n) {
      const CosAlg a = rnd(g, L), b = rnd(g, L), c = rnd(g, L);
      const CosRat k = rnd_s(g), t = rnd_s(g);
      CK(s, cos_alg_valid(L, &a));
      /* (A, +) is an abelian group */
      CK(s, eq(L, ad(s, L, a, b), ad(s, L, b, a)));
      CK(s, eq(L, ad(s, L, ad(s, L, a, b), c), ad(s, L, a, ad(s, L, b, c))));
      CK(s, eq(L, ad(s, L, a, z), a) && zero(L, ad(s, L, a, ng(s, L, a))) && eq(L, sb(s, L, a, b), ad(s, L, a, ng(s, L, b))));
      /* the product is two-sided distributive over + and has the unit e_0 */
      CK(s, eq(L, mu(s, L, a, ad(s, L, b, c)), ad(s, L, mu(s, L, a, b), mu(s, L, a, c))));
      CK(s, eq(L, mu(s, L, ad(s, L, a, b), c), ad(s, L, mu(s, L, a, c), mu(s, L, b, c))));
      CK(s, eq(L, mu(s, L, a, one), a) && eq(L, mu(s, L, one, a), a) && zero(L, mu(s, L, a, z)) && zero(L, mu(s, L, z, a)));
      /* a Q-algebra: rational scalars commute with everything and pass through the product */
      CK(s, eq(L, sc(s, L, k, mu(s, L, a, b)), mu(s, L, sc(s, L, k, a), b)) && eq(L, sc(s, L, k, mu(s, L, a, b)), mu(s, L, a, sc(s, L, k, b))));
      CK(s, eq(L, sc(s, L, k, ad(s, L, a, b)), ad(s, L, sc(s, L, k, a), sc(s, L, k, b))));
      CK(s, eq(L, sc(s, L, qadd(s, k, t), a), ad(s, L, sc(s, L, k, a), sc(s, L, t, a))) && eq(L, sc(s, L, qmul(s, k, t), a), sc(s, L, k, sc(s, L, t, a))));
      CK(s, eq(L, sc(s, L, Q(1, 1), a), a) && eq(L, mu(s, L, real(k), a), sc(s, L, k, a)) && eq(L, mu(s, L, a, real(k)), sc(s, L, k, a)));
    }
  }
}

/* ------------------------------------------------------------------------------- the tower: each level is a subring of the next (Artin 11.3) */
static void test_embed(CosSuite *s, Rng *g) {
  for (uint32_t lo = 0; lo < NLEV; ++lo)
    for (uint32_t hi = lo; hi < NLEV; ++hi)
      for (int n = 0; n < 30; ++n) {
        const CosAlg a = rnd(g, lo), b = rnd(g, lo);
        CosAlg A = sentinel(), B = sentinel(), U = sentinel();
        CK(s, cos_alg_embed(lo, hi, &a, &A) == COS_OK && cos_alg_embed(lo, hi, &b, &B) == COS_OK && cos_alg_valid(hi, &A));
        CK(s, cos_alg_embed(lo, hi, &(CosAlg){0}, &U) == COS_E_INVALID); /* an all-zero struct is not canonical (den 0): refused, not copied */
        CosAlg one_lo = E(lo, 0), one_hi = sentinel();
        CK(s, cos_alg_embed(lo, hi, &one_lo, &one_hi) == COS_OK && eq(hi, one_hi, E(hi, 0))); /* the unit goes to the unit */
        CosAlg ab_lo = mu(s, lo, a, b), ab_hi = sentinel();
        CK(s, cos_alg_embed(lo, hi, &ab_lo, &ab_hi) == COS_OK);
        CK(s, eq(hi, mu(s, hi, A, B), ab_hi));                                         /* multiplicative */
        CosAlg sum_lo = ad(s, lo, a, b), sum_hi = sentinel();
        CK(s, cos_alg_embed(lo, hi, &sum_lo, &sum_hi) == COS_OK && eq(hi, ad(s, hi, A, B), sum_hi)); /* additive */
        CosAlg cj_lo = cj(s, lo, a), cj_hi = sentinel();
        CK(s, cos_alg_embed(lo, hi, &cj_lo, &cj_hi) == COS_OK && eq(hi, cj(s, hi, A), cj_hi));       /* commutes with conj */
        CK(s, qeq(nm(s, hi, A), nm(s, lo, a)));                                                      /* and with the norm */
        for (uint32_t k = cos_alg_dim(lo); k < COS_ALG_MAX_DIM; ++k) CK(s, A.c[k].num == 0 && A.c[k].den == 1); /* nothing appears above the old part */
      }
  CosAlg a = E(2, 1), out = sentinel(), keep = out;
  CK(s, cos_alg_embed(2, 1, &a, &out) == COS_E_INVALID && same(&out, &keep)); /* going down is refused */
  CK(s, cos_alg_embed(0, 4, &a, &out) == COS_E_INVALID && cos_alg_embed(4, 4, &a, &out) == COS_E_INVALID && same(&out, &keep));
}

/* ------------------------------------------------------------------------------------------------- conjugation is an anti-automorphism (Baez 2.2) */
static void test_involution(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    for (int n = 0; n < 40; ++n) {
      const CosAlg a = rnd(g, L), b = rnd(g, L);
      const CosRat k = rnd_s(g);
      CK(s, eq(L, cj(s, L, ad(s, L, a, b)), ad(s, L, cj(s, L, a), cj(s, L, b)))); /* Q-linear */
      CK(s, eq(L, cj(s, L, sc(s, L, k, a)), sc(s, L, k, cj(s, L, a))));
      CK(s, eq(L, cj(s, L, cj(s, L, a)), a));                                      /* an involution */
      CK(s, eq(L, cj(s, L, mu(s, L, a, b)), mu(s, L, cj(s, L, b), cj(s, L, a)))); /* order reversing, at every level including the octonions */
      CosAlg t = ad(s, L, a, cj(s, L, a));
      CK(s, is_real(L, t) && qeq(t.c[0], qmul(s, Q(2, 1), a.c[0])));                /* a + conj(a) = 2 Re(a) is rational */
      CK(s, qeq(mu(s, L, a, b).c[0], mu(s, L, b, a).c[0]));                       /* Re(ab) = Re(ba): the trace is symmetric */
    }
    if (L == 0) {
      const CosAlg a = rnd(g, 0);
      CK(s, eq(0, cj(s, 0, a), a)); /* in Q every element is its own conjugate */
    } else {
      CK(s, !eq(L, cj(s, L, E(L, 1)), E(L, 1)) && eq(L, cj(s, L, E(L, 1)), ng(s, L, E(L, 1)))); /* from level 1 on, A' is never real (Baez prop. 1) */
    }
  }
}

/* ------------------------------------------------------ the norm: composition of squares (Hurwitz), polarization, the frozen integer identities */
static void test_norm(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    for (int n = 0; n < 60; ++n) {
      const CosAlg a = rnd(g, L), b = rnd(g, L);
      const CosRat na = nm(s, L, a), nb = nm(s, L, b);
      const CosAlg aa = mu(s, L, a, cj(s, L, a)), bb = mu(s, L, cj(s, L, a), a);
      CK(s, is_real(L, aa) && qeq(aa.c[0], na) && eq(L, aa, bb));                   /* a conj(a) = conj(a) a = norm2(a) */
      CK(s, qeq(na, dot(s, L, a, a)) && cos_rat_sign(na) >= 0 && (cos_rat_sign(na) > 0 || zero(L, a))); /* a sum of squares: > 0 unless a = 0 */
      CK(s, qeq(nm(s, L, mu(s, L, a, b)), qmul(s, na, nb)));                         /* norm2(ab) = norm2(a) norm2(b) at every level 0..3 */
      CK(s, qeq(nm(s, L, cj(s, L, a)), na) && qeq(nm(s, L, ng(s, L, a)), na));
      /* polarization: <a,b> = Re(a conj(b)) = (norm2(a+b) - norm2(a) - norm2(b)) / 2 is symmetric and agrees with the sum of products */
      const CosRat ip = dot(s, L, a, b);
      CK(s, qeq(mu(s, L, a, cj(s, L, b)).c[0], ip) && qeq(mu(s, L, b, cj(s, L, a)).c[0], ip));
      CK(s, qeq(qmul(s, Q(2, 1), ip), qsub(s, nm(s, L, ad(s, L, a, b)), qadd(s, na, nb))));
    }
  }
  CosAlg z;
  cos_alg_zero(&z);
  CK(s, qeq(nm(s, 3, z), Q(0, 1)));
  /* the frozen integer identities (checked by hand and by the oracle) */
  const int64_t c1[2] = {1, 2}, c2[2] = {3, 4}, q1[4] = {1, 2, 3, 4}, q2[4] = {5, 6, 7, 8}, o1[8] = {1, 2, 3, 4, 5, 6, 7, 8}, o2[8] = {8, 7, 6, 5, 4, 3, 2, 1};
  const int64_t cw[2] = {-5, 10}, qw12[4] = {-60, 12, 30, 24}, qw21[4] = {-60, 20, 14, 32};
  const int64_t ow12[8] = {-104, 14, 12, 10, 152, 42, 4, 74}, ow21[8] = {-104, 32, 48, 64, -64, 60, 112, 56};
  CK(s, eq(1, mu(s, 1, from(1, c1, 2), from(1, c2, 2)), from(1, cw, 2)));                      /* (1+2i)(3+4i) = -5+10i: the two-square identity */
  CK(s, eq(2, mu(s, 2, from(2, q1, 4), from(2, q2, 4)), from(2, qw12, 4)));                    /* the quaternion product: 30 * 174 = 60^2+12^2+30^2+24^2 */
  CK(s, eq(2, mu(s, 2, from(2, q2, 4), from(2, q1, 4)), from(2, qw21, 4)));                    /* and the other order is different: noncommutative */
  CK(s, eq(3, mu(s, 3, from(3, o1, 8), from(3, o2, 8)), from(3, ow12, 8)));                    /* an octonion product, both orders */
  CK(s, eq(3, mu(s, 3, from(3, o2, 8), from(3, o1, 8)), from(3, ow21, 8)));
  CK(s, qeq(nm(s, 3, from(3, ow12, 8)), Q(41616, 1)));                                         /* 204 * 204: the eight-square identity */
  const int64_t ow_inv_num[8] = {1, -2, -3, -4, -5, -6, -7, -8};                                /* inv(o1) = conj(o1) / 204 */
  CK(s, eq(3, iv(s, 3, from(3, o1, 8)), sc(s, 3, Q(1, 204), from(3, ow_inv_num, 8))));
}

/* ------------------------------------------------------------------------------------------------ division: inverses and no zero divisors */
static void test_division(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    const CosAlg one = E(L, 0);
    for (int n = 0; n < 50; ++n) {
      const CosAlg a = rnd_nz(g, L), b = rnd_nz(g, L), x = rnd(g, L);
      const CosAlg ai = iv(s, L, a);
      CK(s, eq(L, mu(s, L, a, ai), one) && eq(L, mu(s, L, ai, a), one));                       /* a two-sided inverse, at every level */
      CK(s, eq(L, iv(s, L, ai), a));
      CK(s, eq(L, ai, sc(s, L, Q(1, 1), ai)));                                                  /* scaling by 1 is the identity */
      CK(s, eq(L, ai, cj(s, L, a)) == qeq(nm(s, L, a), Q(1, 1)));                              /* inv(a) = conj(a) exactly when norm2(a) = 1 */
      CK(s, eq(L, iv(s, L, mu(s, L, a, b)), mu(s, L, iv(s, L, b), ai)));                       /* (ab)^-1 = b^-1 a^-1 */
      /* both divisions are solvable and unique, even for octonions (alternativity; Moufang) */
      CK(s, eq(L, mu(s, L, ai, mu(s, L, a, x)), x) && eq(L, mu(s, L, mu(s, L, x, a), ai), x));
      CK(s, eq(L, mu(s, L, a, mu(s, L, ai, x)), x) && eq(L, mu(s, L, mu(s, L, x, ai), a), x));
      /* no zero divisors: a x = 0 only if a = 0 or x = 0 (norm2(ax) = norm2(a) norm2(x) > 0) */
      CK(s, !zero(L, mu(s, L, a, b)));
    }
    for (uint32_t i = 0; i < cos_alg_dim(L); ++i)
      for (uint32_t j = 0; j < cos_alg_dim(L); ++j) CK(s, !zero(L, mu(s, L, E(L, i), E(L, j))));
    CosAlg z;
    cos_alg_zero(&z);
    CosAlg out = sentinel(), keep = out;
    CK(s, cos_alg_inv(L, &z, &out) == COS_E_INVALID && same(&out, &keep)); /* the inverse of zero is refused and *out is untouched */
  }
}

/* ------------------------------------------- where each law holds and where it stops: commutativity, associativity, alternativity, the centre */
static void test_laws(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    const uint32_t d = cos_alg_dim(L);
    uint32_t nonassoc = 0, noncomm = 0;
    for (int n = 0; n < 60; ++n) {
      const CosAlg a = rnd(g, L), b = rnd(g, L), c = rnd(g, L);
      if (!eq(L, mu(s, L, a, b), mu(s, L, b, a))) ++noncomm;
      if (!zero(L, assoc(s, L, a, b, c))) ++nonassoc;
      /* alternative at every level (the whole tower): (aa)b = a(ab), (ba)a = b(aa); hence flexible (ab)a = a(ba) */
      CK(s, eq(L, mu(s, L, mu(s, L, a, a), b), mu(s, L, a, mu(s, L, a, b))));
      CK(s, eq(L, mu(s, L, mu(s, L, b, a), a), mu(s, L, b, mu(s, L, a, a))));
      CK(s, eq(L, mu(s, L, mu(s, L, a, b), a), mu(s, L, a, mu(s, L, b, a))));
      /* the associator is alternating: it changes sign under each transposition (Baez 1.1) */
      CK(s, eq(L, assoc(s, L, a, b, c), ng(s, L, assoc(s, L, b, a, c))) && eq(L, assoc(s, L, a, b, c), ng(s, L, assoc(s, L, a, c, b))));
      CK(s, eq(L, assoc(s, L, a, b, c), assoc(s, L, b, c, a)));                    /* hence invariant under a cyclic shift */
      /* the Moufang identities, which replace associativity in the octonions */
      CK(s, eq(L, mu(s, L, c, mu(s, L, a, mu(s, L, c, b))), mu(s, L, mu(s, L, mu(s, L, c, a), c), b)));
      CK(s, eq(L, mu(s, L, mu(s, L, mu(s, L, a, c), b), c), mu(s, L, a, mu(s, L, c, mu(s, L, b, c)))));
      CK(s, eq(L, mu(s, L, mu(s, L, c, a), mu(s, L, b, c)), mu(s, L, mu(s, L, c, mu(s, L, a, b)), c)));
      CK(s, eq(L, mu(s, L, mu(s, L, c, a), mu(s, L, b, c)), mu(s, L, c, mu(s, L, mu(s, L, a, b), c))));
      /* power-associative */
      const CosAlg a2 = mu(s, L, a, a);
      CK(s, eq(L, mu(s, L, a2, a), mu(s, L, a, a2)) && eq(L, mu(s, L, mu(s, L, a2, a), a), mu(s, L, a2, a2)));
      /* Emil Artin's theorem (Baez, eq. (1)): two elements generate an associative subalgebra, so words in a and b associate */
      const CosAlg ab = mu(s, L, a, b), ba = mu(s, L, b, a);
      CK(s, zero(L, assoc(s, L, a, b, ab)) && zero(L, assoc(s, L, a, ab, b)) && zero(L, assoc(s, L, ab, a, b)) && zero(L, assoc(s, L, a, b, ba)));
      CK(s, zero(L, assoc(s, L, ab, ba, a)) && zero(L, assoc(s, L, a, ba, ab)) && zero(L, assoc(s, L, cj(s, L, a), b, a)));
    }
    if (L <= 1u) {
      CK(s, noncomm == 0u);                                                        /* commutative: fields */
      for (uint32_t i = 0; i < d; ++i)
        for (uint32_t j = 0; j < d; ++j) CK(s, cos_alg_basis_sign(L, i, j) == cos_alg_basis_sign(L, j, i));
    } else {
      CK(s, noncomm > 0u);
      /* witness: i j = -j i != 0 */
      CK(s, !eq(L, mu(s, L, E(L, 1), E(L, 2)), mu(s, L, E(L, 2), E(L, 1))) && eq(L, mu(s, L, E(L, 1), E(L, 2)), ng(s, L, mu(s, L, E(L, 2), E(L, 1)))));
      /* the centre is Q: every imaginary basis element anticommutes with some other one, and commutators of basis elements land on distinct basis
       * elements, so an element commuting with every basis element has no imaginary part */
      for (uint32_t k = 1; k < d; ++k) {
        bool found = false;
        for (uint32_t j = 1; j < d; ++j)
          if (j != k && cos_alg_basis_sign(L, k, j) != cos_alg_basis_sign(L, j, k)) found = true;
        CK(s, found);
      }
      for (int n = 0; n < 40; ++n) {
        const CosAlg a = rnd(g, L);
        bool commutes_with_all = true;
        for (uint32_t j = 0; j < d; ++j)
          if (!eq(L, mu(s, L, a, E(L, j)), mu(s, L, E(L, j), a))) commutes_with_all = false;
        CK(s, commutes_with_all == is_real(L, a));                                 /* the centre is exactly the rationals */
      }
    }
    if (L <= 2u) {
      CK(s, nonassoc == 0u); /* associative: Q, Q(i) and the quaternions are rings in Artin's sense (without commutativity from level 2) */
    } else {
      CK(s, nonassoc > 0u);
      /* witness: (e1 e2) e4 = - e1 (e2 e4), so the associator is 2 (e1 e2) e4, a nonzero multiple of a basis element */
      CK(s, eq(3, mu(s, 3, mu(s, 3, E(3, 1), E(3, 2)), E(3, 4)), ng(s, 3, mu(s, 3, E(3, 1), mu(s, 3, E(3, 2), E(3, 4))))));
      CK(s, !zero(3, assoc(s, 3, E(3, 1), E(3, 2), E(3, 4))));
      /* on basis elements: the three of a Fano line generate a quaternion subalgebra (associative); any other triple of distinct ones does not */
      for (uint32_t i = 1; i < 8; ++i)
        for (uint32_t j = 1; j < 8; ++j)
          for (uint32_t k = 1; k < 8; ++k) {
            if (i == j || j == k || i == k) continue;
            const bool collinear = (i ^ j) == k;
            CK(s, zero(3, assoc(s, 3, E(3, i), E(3, j), E(3, k))) == collinear);
          }
      /* the frozen associator of two integer octonions with a sparse third */
      const int64_t a8[8] = {1, 2, 3, 4, 5, 6, 7, 8}, b8[8] = {8, 7, 6, 5, 4, 3, 2, 1}, c8[8] = {1, 0, 2, 0, 3, 0, 4, 0}, w[8] = {0, -324, 792, -252, -144, -144, -288, 360};
      CK(s, eq(3, assoc(s, 3, from(3, a8, 8), from(3, b8, 8), from(3, c8, 8)), from(3, w, 8)));
    }
  }
}

/* --------------------------------------------------------- the regular representations: the product as exact linear algebra (Baez 2.3, 18.755 6.2) */
static void test_regrep(CosSuite *s, Rng *g) {
  for (uint32_t L = 0; L < NLEV; ++L) {
    const uint32_t d = cos_alg_dim(L);
    uint32_t hom_fail = 0, commute_fail = 0;
    for (int n = 0; n < 25; ++n) {
      const CosAlg a = rnd(g, L), b = rnd(g, L), x = rnd(g, L);
      const Mat La = lmat(s, L, a), Rb = rmat(s, L, b);
      CK(s, eq(L, mvec(s, L, &La, x), mu(s, L, a, x)) && eq(L, mvec(s, L, &Rb, x), mu(s, L, x, b)));   /* L_a x = a x and R_b x = x b */
      const Mat I = mscalar(d, nm(s, L, a));
      const Mat Lt = mtr(d, &La);
      const Mat LtL = mmul(s, d, &Lt, &La);
      CK(s, meq(d, &LtL, &I));                                                                          /* L_a^T L_a = norm2(a) I: L_a is a similarity */
      const Mat Rt = mtr(d, &Rb);
      const Mat RtR = mmul(s, d, &Rt, &Rb);
      const Mat Ib = mscalar(d, nm(s, L, b));
      CK(s, meq(d, &RtR, &Ib));
      /* imaginary a, b: L_a skew-adjoint, L_a^2 = -norm2(a), and the Clifford relation L_a L_b + L_b L_a = -2 <a,b> */
      const CosAlg p = rnd_im(g, L), q = rnd_im(g, L);
      const Mat Lp = lmat(s, L, p), Lq = lmat(s, L, q);
      const Mat Lpt = mtr(d, &Lp), nLp = mneg(s, d, &Lp);
      CK(s, meq(d, &Lpt, &nLp));
      const Mat Lp2 = mmul(s, d, &Lp, &Lp), nI = mscalar(d, qsub(s, Q(0, 1), nm(s, L, p)));
      CK(s, meq(d, &Lp2, &nI));
      const Mat LpLq = mmul(s, d, &Lp, &Lq), LqLp = mmul(s, d, &Lq, &Lp), anti = madd(s, d, &LpLq, &LqLp);
      const Mat cliff = mscalar(d, qsub(s, Q(0, 1), qmul(s, Q(2, 1), dot(s, L, p, q))));
      CK(s, meq(d, &anti, &cliff));
      /* a -> L_a is multiplicative exactly where the algebra is associative; left and right translations commute exactly there too */
      const Mat Lab = lmat(s, L, mu(s, L, a, b)), Lb = lmat(s, L, b), LaLb = mmul(s, d, &La, &Lb);
      if (!meq(d, &Lab, &LaLb)) ++hom_fail;
      const Mat LaRb = mmul(s, d, &La, &Rb), RbLa = mmul(s, d, &Rb, &La);
      if (!meq(d, &LaRb, &RbLa)) ++commute_fail;
    }
    if (L <= 2u) CK(s, hom_fail == 0u && commute_fail == 0u);
    else CK(s, hom_fail > 0u && commute_fail > 0u); /* at level 3 L_(ab) != L_a L_b: the representation trick needs only that each L_a is Q-linear */
  }
  /* range checks */
  const CosAlg a = E(2, 1);
  CosRat out = {5, 7};
  CK(s, cos_alg_lmat_entry(2, &a, 4, 0, &out) == COS_E_INVALID && cos_alg_lmat_entry(2, &a, 0, 4, &out) == COS_E_INVALID && cos_alg_lmat_entry(4, &a, 0, 0, &out) == COS_E_INVALID);
  CK(s, cos_alg_rmat_entry(2, &a, 4, 0, &out) == COS_E_INVALID && cos_alg_rmat_entry(2, &a, 0, 4, &out) == COS_E_INVALID && out.num == 5 && out.den == 7);
}

/* ------------------------------------------------------------------------------------------------------------------------------------ symmetry */
static CosAlg unit_quat(CosSuite *s, Rng *g) { /* a rational unit quaternion: a product of 1 to 3 of these (Hurwitz units and Pythagorean quadruples) */
  static const int64_t G[7][5] = {{1, 1, 1, 1, 2}, {1, -1, 1, -1, 2}, {1, 2, 2, 4, 5}, {1, 2, 2, 0, 3}, {0, 4, 8, 1, 9}, {2, 3, 6, 0, 7}, {4, 4, 7, 0, 9}};
  CosAlg q = E(2, 0);
  for (int n = rr(g, 1, 3); n > 0; --n) {
    const int64_t *r = G[rr(g, 0, 6)];
    CosAlg u;
    cos_alg_zero(&u);
    for (uint32_t k = 0; k < 4; ++k) u.c[k] = Q(r[k], r[4]);
    CK(s, qeq(nm(s, 2, u), Q(1, 1)));
    q = mu(s, 2, q, u);
  }
  CK(s, qeq(nm(s, 2, q), Q(1, 1)));
  return q;
}
static CosAlg conj_by(CosSuite *s, uint32_t L, CosAlg q, CosAlg x) { return mu(s, L, mu(s, L, q, x), iv(s, L, q)); } /* q x q^-1 */
static CosRat det3(CosSuite *s, const Mat *R) { /* the 3 x 3 determinant by the triple product */
  const CosRat(*m)[COS_ALG_MAX_DIM] = R->m;
  CosRat t1 = qmul(s, m[0][0], qsub(s, qmul(s, m[1][1], m[2][2]), qmul(s, m[1][2], m[2][1])));
  CosRat t2 = qmul(s, m[0][1], qsub(s, qmul(s, m[1][0], m[2][2]), qmul(s, m[1][2], m[2][0])));
  CosRat t3 = qmul(s, m[0][2], qsub(s, qmul(s, m[1][0], m[2][1]), qmul(s, m[1][1], m[2][0])));
  return qadd(s, qsub(s, t1, t2), t3);
}
static Mat rot3(CosSuite *s, CosAlg q) { /* the matrix of x -> q x q^-1 on span(i, j, k): column c is the image of e_{c+1} */
  Mat R = mzero();
  for (uint32_t c = 0; c < 3; ++c) {
    const CosAlg img = conj_by(s, 2, q, E(2, c + 1));
    CK(s, qeq(img.c[0], Q(0, 1)));
    for (uint32_t r = 0; r < 3; ++r) R.m[r][c] = img.c[r + 1];
  }
  return R;
}
static void split(CosAlg x, CosAlg *lo, CosAlg *hi) {
  cos_alg_zero(lo);
  cos_alg_zero(hi);
  for (uint32_t k = 0; k < 4; ++k) {
    lo->c[k] = x.c[k];
    hi->c[k] = x.c[k + 4];
  }
}
static CosAlg join(CosAlg lo, CosAlg hi) {
  CosAlg x;
  cos_alg_zero(&x);
  for (uint32_t k = 0; k < 4; ++k) {
    x.c[k] = lo.c[k];
    x.c[k + 4] = hi.c[k];
  }
  return x;
}
static CosAlg so4(CosSuite *s, CosAlg p, CosAlg q, CosAlg x) { /* the SO(4) inside G2: (a, b) -> (p a p^-1, q b p^-1) */
  CosAlg lo, hi;
  split(x, &lo, &hi);
  return join(conj_by(s, 2, p, lo), mu(s, 2, mu(s, 2, q, hi), iv(s, 2, p)));
}

static void test_symmetry(CosSuite *s, Rng *g) {
  /* level 2: x -> q x q^-1 for q != 0 is an automorphism of H over Q that fixes Re and norm2: the exact rational spin homomorphism (Artin 9.4.1) */
  for (int n = 0; n < 60; ++n) {
    const CosAlg q = rnd_nz(g, 2), p = rnd_nz(g, 2), x = rnd(g, 2), y = rnd(g, 2);
    CK(s, eq(2, conj_by(s, 2, q, mu(s, 2, x, y)), mu(s, 2, conj_by(s, 2, q, x), conj_by(s, 2, q, y))));  /* multiplicative */
    CK(s, eq(2, conj_by(s, 2, q, ad(s, 2, x, y)), ad(s, 2, conj_by(s, 2, q, x), conj_by(s, 2, q, y)))); /* additive */
    CK(s, eq(2, conj_by(s, 2, q, E(2, 0)), E(2, 0)));
    CK(s, qeq(nm(s, 2, conj_by(s, 2, q, x)), nm(s, 2, x)) && qeq(conj_by(s, 2, q, x).c[0], x.c[0]));      /* norm and real part preserved */
    CK(s, eq(2, conj_by(s, 2, mu(s, 2, q, p), x), conj_by(s, 2, q, conj_by(s, 2, p, x))));                /* C_(qp) = C_q after C_p: a homomorphism */
    CK(s, eq(2, conj_by(s, 2, sc(s, 2, Q(3, 2), q), x), conj_by(s, 2, q, x))); /* scaling q does not change the action */
    /* the kernel is exactly the centre: a real q acts as the identity, a non-real q moves some basis element */
    const CosAlg r = real(Q(rr(g, 1, 9), rr(g, 1, 5)));
    CK(s, eq(2, conj_by(s, 2, r, x), x));
    if (!is_real(2, q)) {
      bool moved = false;
      for (uint32_t j = 1; j < 4; ++j)
        if (!eq(2, conj_by(s, 2, q, E(2, j)), E(2, j))) moved = true;
      CK(s, moved);
    }
  }
  CK(s, eq(2, conj_by(s, 2, E(2, 1), E(2, 2)), ng(s, 2, E(2, 2))) && eq(2, conj_by(s, 2, E(2, 1), E(2, 1)), E(2, 1))); /* i turns j into -j: a half turn about i */
  /* the 3 x 3 matrix of the action is a rational rotation: R^T R = I, det R = 1, R_(qp) = R_q R_p, R_(-q) = R_q (Artin 9.4.1, Lemma 9.4.5) */
  for (int n = 0; n < 40; ++n) {
    const CosAlg q = rnd_nz(g, 2), p = rnd_nz(g, 2);
    const Mat Rq = rot3(s, q), Rp = rot3(s, p), Rqp = rot3(s, mu(s, 2, q, p)), Rnq = rot3(s, ng(s, 2, q));
    const Mat Rt = mtr(3, &Rq), RtR = mmul(s, 3, &Rt, &Rq), I3 = mscalar(3, Q(1, 1)), RR = mmul(s, 3, &Rq, &Rp);
    CK(s, meq(3, &RtR, &I3) && qeq(det3(s, &Rq), Q(1, 1)) && meq(3, &Rqp, &RR) && meq(3, &Rnq, &Rq));
    const CosAlg v = rnd_im(g, 2);
    CosAlg Rv;   /* the 3 x 3 block acts on the imaginary part (components 1..3) of v, whose real part is 0 */
    cos_alg_zero(&Rv);
    for (uint32_t r = 0; r < 3; ++r) {
      CosRat acc = {0, 1};
      for (uint32_t c = 0; c < 3; ++c) acc = qadd(s, acc, qmul(s, Rq.m[r][c], v.c[c + 1]));
      Rv.c[r + 1] = acc;
    }
    CK(s, eq(2, Rv, conj_by(s, 2, q, v)));
  }
  /* Artin Lemma 9.4.4: for imaginary u, v: u v = -(u.v) + u x v, with the usual cross product; hence uv + vu = -2 u.v */
  for (int n = 0; n < 40; ++n) {
    const CosAlg u = rnd_im(g, 2), v = rnd_im(g, 2), uv = mu(s, 2, u, v);
    CK(s, qeq(uv.c[0], qsub(s, Q(0, 1), dot(s, 2, u, v))));
    CK(s, qeq(uv.c[1], qsub(s, qmul(s, u.c[2], v.c[3]), qmul(s, u.c[3], v.c[2]))));
    CK(s, qeq(uv.c[2], qsub(s, qmul(s, u.c[3], v.c[1]), qmul(s, u.c[1], v.c[3]))));
    CK(s, qeq(uv.c[3], qsub(s, qmul(s, u.c[1], v.c[2]), qmul(s, u.c[2], v.c[1]))));
  }
  /* level 3 (Baez 4.1): a x b = (ab - ba)/2 for imaginary a, b: ab = -<a,b> + a x b, |a x b|^2 + <a,b>^2 = |a|^2 |b|^2, a x b is orthogonal to a and b */
  for (int n = 0; n < 40; ++n) {
    const CosAlg a = rnd_im(g, 3), b = rnd_im(g, 3);
    const CosAlg c = sc(s, 3, Q(1, 2), sb(s, 3, mu(s, 3, a, b), mu(s, 3, b, a)));
    CK(s, eq(3, mu(s, 3, a, b), ad(s, 3, real(qsub(s, Q(0, 1), dot(s, 3, a, b))), c)));
    CK(s, qeq(qadd(s, nm(s, 3, c), qmul(s, dot(s, 3, a, b), dot(s, 3, a, b))), qmul(s, nm(s, 3, a), nm(s, 3, b))));
    CK(s, qeq(dot(s, 3, c, a), Q(0, 1)) && qeq(dot(s, 3, c, b), Q(0, 1)) && qeq(c.c[0], Q(0, 1)));
  }
  /* the SO(4) inside G2 = Aut(O): (a, b) -> (p a p^-1, q b p^-1) for unit quaternions p, q is an automorphism of the octonions that fixes the norm */
  for (int n = 0; n < 40; ++n) {
    const CosAlg p = unit_quat(s, g), q = unit_quat(s, g), x = rnd(g, 3), y = rnd(g, 3);
    CK(s, eq(3, so4(s, p, q, mu(s, 3, x, y)), mu(s, 3, so4(s, p, q, x), so4(s, p, q, y))));
    CK(s, eq(3, so4(s, p, q, E(3, 0)), E(3, 0)) && qeq(nm(s, 3, so4(s, p, q, x)), nm(s, 3, x)));
    CK(s, eq(3, so4(s, p, q, ad(s, 3, x, y)), ad(s, 3, so4(s, p, q, x), so4(s, p, q, y))));
  }
  /* but conjugation x -> u x u^-1 in the octonions is well defined (flexibility) and, in general, NOT an automorphism: G2 is not a group of conjugations */
  uint32_t not_aut = 0;
  for (int n = 0; n < 30; ++n) {
    const CosAlg u = rnd_nz(g, 3), x = rnd(g, 3), y = rnd(g, 3), ui = iv(s, 3, u);
    CK(s, eq(3, mu(s, 3, mu(s, 3, u, x), ui), mu(s, 3, u, mu(s, 3, x, ui))));
    if (!eq(3, conj_by(s, 3, u, mu(s, 3, x, y)), mu(s, 3, conj_by(s, 3, u, x), conj_by(s, 3, u, y)))) ++not_aut;
  }
  CK(s, not_aut > 0u);
}

/* ----------------------------------------------------------------------------------------------------- errors, aliasing, overflow, the level limit */
static void test_contract(CosSuite *s, Rng *g) {
  /* level 4 and above are refused by every operation, and *out stays as it was */
  const CosAlg a = E(3, 1), b = E(3, 2);
  CosAlg out = sentinel();
  const CosAlg keep = out;
  CosRat qo = {123, 7};
  for (uint32_t L = 4; L < 7; L += 2) {
    CK(s, cos_alg_add(L, &a, &b, &out) == COS_E_INVALID && cos_alg_sub(L, &a, &b, &out) == COS_E_INVALID && cos_alg_mul(L, &a, &b, &out) == COS_E_INVALID);
    CK(s, cos_alg_neg(L, &a, &out) == COS_E_INVALID && cos_alg_conj(L, &a, &out) == COS_E_INVALID && cos_alg_inv(L, &a, &out) == COS_E_INVALID);
    CK(s, cos_alg_scale(L, Q(1, 2), &a, &out) == COS_E_INVALID && cos_alg_one(L, &out) == COS_E_INVALID && cos_alg_basis(L, 0, &out) == COS_E_INVALID);
    CK(s, cos_alg_from_ints(L, NULL, 0, &out) == COS_E_INVALID && cos_alg_norm2(L, &a, &qo) == COS_E_INVALID && qo.num == 123 && same(&out, &keep));
    CK(s, !cos_alg_valid(L, &a) && !cos_alg_eq(L, &a, &a) && !cos_alg_is_zero(L, &a));
  }
  /* NULL pointers */
  CK(s, cos_alg_mul(2, NULL, &b, &out) == COS_E_INVALID && cos_alg_mul(2, &a, NULL, &out) == COS_E_INVALID && cos_alg_mul(2, &a, &b, NULL) == COS_E_INVALID);
  CK(s, cos_alg_add(2, NULL, &b, &out) == COS_E_INVALID && cos_alg_neg(2, NULL, &out) == COS_E_INVALID && cos_alg_conj(2, &a, NULL) == COS_E_INVALID);
  CK(s, cos_alg_inv(2, NULL, &out) == COS_E_INVALID && cos_alg_norm2(2, &a, NULL) == COS_E_INVALID && cos_alg_scale(2, Q(1, 1), NULL, &out) == COS_E_INVALID);
  CK(s, !cos_alg_valid(2, NULL) && same(&out, &keep));
  cos_alg_zero(NULL); /* a void function cannot report a NULL, but it must not fault */
  CK(s, !cos_alg_eq(2, NULL, &b) && !cos_alg_eq(2, &a, NULL) && !cos_alg_is_zero(2, NULL));
  CK(s, cos_alg_one(2, NULL) == COS_E_INVALID && cos_alg_basis(2, 0, NULL) == COS_E_INVALID && cos_alg_embed(1, 2, &a, NULL) == COS_E_INVALID);
  CK(s, cos_alg_lmat_entry(2, &a, 0, 0, NULL) == COS_E_INVALID && cos_alg_rmat_entry(2, NULL, 0, 0, &qo) == COS_E_INVALID && qo.num == 123);
  /* operands that are not canonical: an unreduced rational, a zero denominator, a nonzero unused component */
  CosAlg bad = E(2, 1);
  bad.c[1] = (CosRat){2, 4};
  CK(s, !cos_alg_valid(2, &bad) && cos_alg_mul(2, &bad, &bad, &out) == COS_E_INVALID && cos_alg_add(2, &bad, &bad, &out) == COS_E_INVALID);
  CK(s, cos_alg_conj(2, &bad, &out) == COS_E_INVALID && cos_alg_norm2(2, &bad, &qo) == COS_E_INVALID && cos_alg_inv(2, &bad, &out) == COS_E_INVALID);
  CosAlg bad2 = E(2, 1);
  bad2.c[5] = Q(1, 1); /* component 5 is not part of a quaternion */
  CK(s, !cos_alg_valid(2, &bad2) && cos_alg_valid(3, &bad2) && cos_alg_mul(2, &bad2, &b, &out) == COS_E_INVALID && cos_alg_mul(2, &b, &bad2, &out) == COS_E_INVALID);
  CK(s, !cos_alg_eq(2, &bad2, &bad2) && !cos_alg_is_zero(2, &bad2) && cos_alg_scale(2, (CosRat){1, 0}, &a, &out) == COS_E_INVALID && same(&out, &keep));
  CK(s, cos_alg_basis(2, 4, &out) == COS_E_INVALID && cos_alg_basis(3, 7, &out) == COS_OK && cos_alg_basis(3, 8, &out) == COS_E_INVALID);
  out = keep;
  /* from_ints */
  const int64_t v9[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9}, vmin[1] = {INT64_MIN}, v2[2] = {5, -6};
  CK(s, cos_alg_from_ints(3, v9, 9, &out) == COS_E_INVALID && cos_alg_from_ints(3, NULL, 1, &out) == COS_E_INVALID && cos_alg_from_ints(3, vmin, 1, &out) == COS_E_RANGE);
  CK(s, same(&out, &keep));
  CK(s, cos_alg_from_ints(3, NULL, 0, &out) == COS_OK && cos_alg_is_zero(3, &out));
  CK(s, cos_alg_from_ints(3, v2, 2, &out) == COS_OK && out.c[0].num == 5 && out.c[1].num == -6 && out.c[2].num == 0 && out.c[2].den == 1 && cos_alg_valid(3, &out));
  /* overflow is refused, never rounded, and the output (also when it aliases an input) is untouched */
  CosAlg big;
  cos_alg_zero(&big);
  big.c[0] = Q(MAXV, 1);
  big.c[1] = Q(MAXV, 1);
  CosAlg two = E(3, 0);
  two.c[0] = Q(2, 1);
  out = keep;
  CK(s, cos_alg_mul(3, &big, &two, &out) == COS_E_RANGE && cos_alg_add(3, &big, &big, &out) == COS_E_RANGE && cos_alg_norm2(3, &big, &qo) == COS_E_RANGE);
  CK(s, cos_alg_scale(3, Q(2, 1), &big, &out) == COS_E_RANGE && cos_alg_inv(3, &big, &out) == COS_E_RANGE && cos_alg_mul(3, &big, &big, &out) == COS_E_RANGE);
  CK(s, same(&out, &keep) && qo.num == 123 && qo.den == 7);
  CosAlg alias = big;
  const CosAlg alias_keep = alias;
  CK(s, cos_alg_mul(3, &alias, &two, &alias) == COS_E_RANGE && same(&alias, &alias_keep)); /* out aliases an input: still untouched on error */
  CK(s, cos_alg_add(3, &alias, &alias, &alias) == COS_E_RANGE && same(&alias, &alias_keep));
  {
    /* every product fits but their SUM does not, so it is the accumulation step that must refuse (not the multiplication step):
     * (2^31 + 2^31 i)(2^31 - 2^31 i) has real part 2^62 + 2^62 = 2^63 > INT64_MAX */
    CosAlg a2, b2;
    cos_alg_zero(&a2);
    cos_alg_zero(&b2);
    a2.c[0] = Q(INT64_C(1) << 31, 1);
    a2.c[1] = Q(INT64_C(1) << 31, 1);
    b2.c[0] = Q(INT64_C(1) << 31, 1);
    b2.c[1] = Q(-(INT64_C(1) << 31), 1);
    out = keep;
    CK(s, cos_alg_mul(1, &a2, &b2, &out) == COS_E_RANGE && same(&out, &keep));
    /* the same for denominators: 1/(p q) - 1/(r s) with four coprime numbers near 2^31 needs a 124-bit common denominator, each product's fits */
    a2.c[0] = Q(1, 2147483647);
    a2.c[1] = Q(1, 2147483587);
    b2.c[0] = Q(1, 2147483629);
    b2.c[1] = Q(1, 2147483579);
    out = keep;
    CK(s, cos_alg_mul(1, &a2, &b2, &out) == COS_E_RANGE && same(&out, &keep));
    CosAlg t2 = a2;
    CK(s, cos_alg_mul(1, &t2, &b2, &t2) == COS_E_RANGE && same(&t2, &a2)); /* the same through an aliased output */
  }
  /* a result that fits is returned exactly, close to the limit: MAXV * 1 */
  CosAlg one = E(3, 0);
  CK(s, cos_alg_mul(3, &big, &one, &out) == COS_OK && eq(3, out, big));
  /* aliasing works when nothing overflows: out may be a, b or both */
  for (uint32_t L = 0; L < NLEV; ++L) {
    const CosAlg x = rnd_nz(g, L), y = rnd(g, L);
    CosAlg t = x, want = mu(s, L, x, y);
    CK(s, cos_alg_mul(L, &t, &y, &t) == COS_OK && eq(L, t, want));
    t = y;
    CK(s, cos_alg_mul(L, &x, &t, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = mu(s, L, x, x);
    CK(s, cos_alg_mul(L, &t, &t, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = ad(s, L, x, y);
    CK(s, cos_alg_add(L, &t, &y, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = iv(s, L, x);
    CK(s, cos_alg_inv(L, &t, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = cj(s, L, x);
    CK(s, cos_alg_conj(L, &t, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = ng(s, L, x);
    CK(s, cos_alg_neg(L, &t, &t) == COS_OK && eq(L, t, want));
    t = x;
    want = sc(s, L, Q(3, 5), x);
    CK(s, cos_alg_scale(L, Q(3, 5), &t, &t) == COS_OK && eq(L, t, want));
  }
}

int main(void) {
  CosSuite s = {"cos_alg", 0, 0};
  Rng g = {0x20261105C05A1900ull};
  test_signs(&s);
  test_ring(&s, &g);
  test_embed(&s, &g);
  test_involution(&s, &g);
  test_norm(&s, &g);
  test_division(&s, &g);
  test_laws(&s, &g);
  test_regrep(&s, &g);
  test_symmetry(&s, &g);
  test_contract(&s, &g);
  return suite_done(&s);
}
