#include "cos_alg.h"

#include "cos_limits.h"

COS_STATIC_ASSERT(sizeof(CosAlg) == 128u, "an element is 8 exact rationals of 16 bytes");
COS_STATIC_ASSERT(COS_ALG_MAX_DIM == (1u << COS_ALG_MAX_LEVEL), "dimension is 2 to the level");

uint32_t cos_alg_dim(uint32_t level) { return level <= COS_ALG_MAX_LEVEL ? (1u << level) : 0u; }

/* Conjugation sign of the basis element e_k: the real unit is its own conjugate, every imaginary unit changes sign. */
static int chi(uint32_t k) { return k == 0u ? 1 : -1; }

/* The sign of e_i e_j, i, j < 2^level. Unrolled from the Cayley-Dickson rule (a,b)(c,d) = (ac - conj(d) b, da + b conj(c)) applied to the basis
 * elements e_i = (e_i', 0) or (0, e_i') with h = 2^(level-1), i = i' + h ib, j = j' + h jb (ib, jb are the top bits). Writing s for the sign of the
 * lower level product, the four cases are
 *     ib jb    product                       sign                       next pair
 *     0  0     (e_i' e_j', 0)                 s(i', j')                  (i', j')
 *     0  1     (0, e_j' e_i')                 s(j', i')                  (j', i')
 *     1  0     (0, e_i' conj(e_j'))           chi(j') s(i', j')          (i', j')
 *     1  1     (-conj(e_j') e_i', 0)         -chi(j') s(j', i')          (j', i')
 * so one pass from the top level to level 0 gives the sign: multiply in the factor of the case (only when ib = 1), and swap the pair when jb = 1.
 * No recursion: three iterations at most. */
static int sgn(uint32_t level, uint32_t i, uint32_t j) {
  int s = 1;
  for (uint32_t l = level; l > 0u; --l) {
    const uint32_t h = 1u << (l - 1u);
    const bool ib = (i & h) != 0u;
    const bool jb = (j & h) != 0u;
    const uint32_t lo_i = i & (h - 1u);
    const uint32_t lo_j = j & (h - 1u);
    if (ib) s *= jb ? -chi(lo_j) : chi(lo_j);
    i = jb ? lo_j : lo_i;
    j = jb ? lo_i : lo_j;
  }
  return s;
}

int cos_alg_basis_sign(uint32_t level, uint32_t i, uint32_t j) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || i >= d || j >= d) return 0;
  return sgn(level, i, j);
}

bool cos_alg_valid(uint32_t level, const CosAlg *a) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || a == NULL) return false;
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) {
    if (k < d) {
      if (!cos_rat_valid(a->c[k])) return false;
    } else if (a->c[k].num != 0 || a->c[k].den != 1) {
      return false;
    }
  }
  return true;
}

void cos_alg_zero(CosAlg *out) {
  if (out == NULL) return; /* a void function cannot report it, but it must not fault: the Forbidden Zone is fail-closed, never a crash */
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) {
    out->c[k].num = 0;
    out->c[k].den = 1;
  }
}

CosStatus cos_alg_one(uint32_t level, CosAlg *out) { return cos_alg_basis(level, 0u, out); }

CosStatus cos_alg_basis(uint32_t level, uint32_t i, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || i >= d || out == NULL) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  r.c[i].num = 1;
  *out = r;
  return COS_OK;
}

CosStatus cos_alg_from_ints(uint32_t level, const int64_t *v, size_t n, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || n > d || (n > 0u && v == NULL)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  for (size_t k = 0; k < n; ++k) {
    const CosStatus st = cos_rat_int(v[k], &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

/* Canonical form is unique (reduced fractions, den > 0), so equality is equality of the fields. */
bool cos_alg_eq(uint32_t level, const CosAlg *a, const CosAlg *b) {
  if (!cos_alg_valid(level, a) || !cos_alg_valid(level, b)) return false;
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) {
    if (a->c[k].num != b->c[k].num || a->c[k].den != b->c[k].den) return false;
  }
  return true;
}

bool cos_alg_is_zero(uint32_t level, const CosAlg *a) {
  if (!cos_alg_valid(level, a)) return false;
  for (uint32_t k = 0; k < COS_ALG_MAX_DIM; ++k) {
    if (a->c[k].num != 0) return false;
  }
  return true;
}

/* Component-wise sum or difference. The result is built in a local so that an error leaves *out alone and out may alias a or b. */
static CosStatus addsub(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out, bool sub) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_alg_valid(level, a) || !cos_alg_valid(level, b)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  for (uint32_t k = 0; k < d; ++k) {
    const CosStatus st = sub ? cos_rat_sub(a->c[k], b->c[k], &r.c[k]) : cos_rat_add(a->c[k], b->c[k], &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

CosStatus cos_alg_add(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out) { return addsub(level, a, b, out, false); }
CosStatus cos_alg_sub(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out) { return addsub(level, a, b, out, true); }

CosStatus cos_alg_neg(uint32_t level, const CosAlg *a, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_alg_valid(level, a)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  for (uint32_t k = 0; k < d; ++k) {
    const CosStatus st = cos_rat_neg(a->c[k], &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

CosStatus cos_alg_scale(uint32_t level, CosRat s, const CosAlg *a, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_rat_valid(s) || !cos_alg_valid(level, a)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  for (uint32_t k = 0; k < d; ++k) {
    const CosStatus st = cos_rat_mul(s, a->c[k], &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

/* The product: c_{i xor j} += sign(i, j) a_i b_j over all pairs. Exact arithmetic makes the order of the additions irrelevant to the value; a zero
 * factor is skipped (a pure speed-up for the sparse elements the engine mostly holds: it cannot change the sum). A refusal can only come from an
 * exact intermediate that does not fit 64 bits. */
CosStatus cos_alg_mul(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_alg_valid(level, a) || !cos_alg_valid(level, b)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  for (uint32_t i = 0; i < d; ++i) {
    if (a->c[i].num == 0) continue;
    for (uint32_t j = 0; j < d; ++j) {
      if (b->c[j].num == 0) continue;
      CosRat p;
      CosStatus st = cos_rat_mul(a->c[i], b->c[j], &p);
      if (st != COS_OK) return st;
      if (sgn(level, i, j) < 0) {
        st = cos_rat_neg(p, &p);
        if (st != COS_OK) return st;
      }
      CosRat *slot = &r.c[i ^ j];
      st = cos_rat_add(*slot, p, slot);
      if (st != COS_OK) return st;
    }
  }
  *out = r;
  return COS_OK;
}

CosStatus cos_alg_conj(uint32_t level, const CosAlg *a, CosAlg *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_alg_valid(level, a)) return COS_E_INVALID;
  CosAlg r;
  cos_alg_zero(&r);
  r.c[0] = a->c[0];
  for (uint32_t k = 1; k < d; ++k) {
    const CosStatus st = cos_rat_neg(a->c[k], &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

/* a conj(a) = sum of the squares of the components, for every level (the signs of conj cancel the signs of the basis products on the diagonal). */
CosStatus cos_alg_norm2(uint32_t level, const CosAlg *a, CosRat *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || !cos_alg_valid(level, a)) return COS_E_INVALID;
  CosRat acc = {0, 1};
  for (uint32_t k = 0; k < d; ++k) {
    CosRat sq;
    CosStatus st = cos_rat_mul(a->c[k], a->c[k], &sq);
    if (st != COS_OK) return st;
    st = cos_rat_add(acc, sq, &acc);
    if (st != COS_OK) return st;
  }
  *out = acc;
  return COS_OK;
}

/* inv(a) = conj(a) / norm2(a), each component divided exactly. norm2(a) = 0 only for a = 0 (a sum of squares of rationals). */
CosStatus cos_alg_inv(uint32_t level, const CosAlg *a, CosAlg *out) {
  CosRat n2;
  CosStatus st = cos_alg_norm2(level, a, &n2);
  if (st != COS_OK) return st;
  if (n2.num == 0) return COS_E_INVALID;
  const uint32_t d = cos_alg_dim(level);
  CosAlg r;
  cos_alg_zero(&r);
  for (uint32_t k = 0; k < d; ++k) {
    CosRat v = a->c[k];
    if (k > 0u) {
      st = cos_rat_neg(v, &v);
      if (st != COS_OK) return st;
    }
    st = cos_rat_div(v, n2, &r.c[k]);
    if (st != COS_OK) return st;
  }
  *out = r;
  return COS_OK;
}

/* Basis elements of the lower level keep their index in the upper one, so inclusion is a plain copy; what has to be checked is only the order of the
 * levels and that the operand is canonical at the level it comes from. */
CosStatus cos_alg_embed(uint32_t from, uint32_t to, const CosAlg *a, CosAlg *out) {
  if (cos_alg_dim(from) == 0u || cos_alg_dim(to) == 0u || from > to || out == NULL || !cos_alg_valid(from, a)) return COS_E_INVALID;
  *out = *a;
  return COS_OK;
}

CosStatus cos_alg_lmat_entry(uint32_t level, const CosAlg *a, uint32_t row, uint32_t col, CosRat *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || row >= d || col >= d || !cos_alg_valid(level, a)) return COS_E_INVALID;
  const uint32_t i = row ^ col; /* e_i e_col = sign e_row */
  if (sgn(level, i, col) > 0) {
    *out = a->c[i];
    return COS_OK;
  }
  return cos_rat_neg(a->c[i], out);
}

CosStatus cos_alg_rmat_entry(uint32_t level, const CosAlg *b, uint32_t row, uint32_t col, CosRat *out) {
  const uint32_t d = cos_alg_dim(level);
  if (d == 0u || out == NULL || row >= d || col >= d || !cos_alg_valid(level, b)) return COS_E_INVALID;
  const uint32_t j = row ^ col; /* e_col e_j = sign e_row */
  if (sgn(level, col, j) > 0) {
    *out = b->c[j];
    return COS_OK;
  }
  return cos_rat_neg(b->c[j], out);
}
