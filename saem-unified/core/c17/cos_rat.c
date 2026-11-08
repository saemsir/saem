#include "cos_rat.h"

#define RMAX INT64_MAX

static uint64_t mag(int64_t v) { return v < 0 ? (uint64_t)(-v) : (uint64_t)v; } /* v is never INT64_MIN */

static uint64_t gcd_u(uint64_t a, uint64_t b) {
  while (b != 0u) {
    const uint64_t t = a % b;
    a = b;
    b = t;
  }
  return a;
}

/* checked multiplication on the symmetric range [-INT64_MAX, INT64_MAX]; true means overflow */
static bool mul_ovf(int64_t a, int64_t b, int64_t *r) {
  if (a == 0 || b == 0) {
    *r = 0;
    return false;
  }
  const uint64_t ua = mag(a), ub = mag(b);
  if (ua > (uint64_t)RMAX / ub) return true;
  const uint64_t p = ua * ub;
  *r = ((a < 0) != (b < 0)) ? -(int64_t)p : (int64_t)p;
  return false;
}

/* 64 x 64 -> 128 bits, in 32-bit limbs */
static void mul128(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo) {
  const uint64_t a0 = a & 0xFFFFFFFFu, a1 = a >> 32, b0 = b & 0xFFFFFFFFu, b1 = b >> 32;
  const uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  const uint64_t mid = (p00 >> 32) + (p01 & 0xFFFFFFFFu) + (p10 & 0xFFFFFFFFu);
  *lo = (p00 & 0xFFFFFFFFu) | (mid << 32);
  *hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
}

typedef struct U128 {
  uint64_t hi;
  uint64_t lo;
} U128;

static U128 mul_u128(uint64_t a, uint64_t b) {
  U128 r;
  mul128(a, b, &r.hi, &r.lo);
  return r;
}

static int cmp128(U128 a, U128 b) {
  if (a.hi != b.hi) return a.hi < b.hi ? -1 : 1;
  if (a.lo != b.lo) return a.lo < b.lo ? -1 : 1;
  return 0;
}

static U128 add128(U128 a, U128 b) {
  U128 r;
  r.lo = a.lo + b.lo;
  r.hi = a.hi + b.hi + (r.lo < a.lo ? 1u : 0u);
  return r;
}

static U128 sub128(U128 a, U128 b) {
  U128 r;
  r.lo = a.lo - b.lo;
  r.hi = a.hi - b.hi - (a.lo < b.lo ? 1u : 0u);
  return r;
}

static uint64_t bit128(U128 a, unsigned int bit) {
  return bit >= 64u ? (a.hi >> (bit - 64u)) & 1u : (a.lo >> bit) & 1u;
}

/* d is a positive CosRat divisor <= INT64_MAX, so doubling rem cannot overflow. */
static U128 divmod128_u64(U128 n, uint64_t d, uint64_t *rem_out) {
  U128 q = {0, 0};
  uint64_t rem = 0;
  for (int bit = 127; bit >= 0; --bit) {
    rem = rem * 2u + bit128(n, (unsigned int)bit);
    if (rem >= d) {
      rem -= d;
      if (bit >= 64) q.hi |= (uint64_t)1u << (unsigned int)(bit - 64);
      else q.lo |= (uint64_t)1u << (unsigned int)bit;
    }
  }
  *rem_out = rem;
  return q;
}

bool cos_rat_valid(CosRat a) {
  if (a.den <= 0 || a.num == INT64_MIN) return false;
  return gcd_u(mag(a.num), (uint64_t)a.den) == 1u; /* gcd(0, d) = d, so zero is valid only as 0/1 */
}

CosStatus cos_rat_make(int64_t num, int64_t den, CosRat *out) {
  if (den == 0) return COS_E_INVALID;
  if (num == INT64_MIN || den == INT64_MIN) return COS_E_RANGE;
  if (den < 0) {
    num = -num;
    den = -den;
  }
  const uint64_t g = gcd_u(mag(num), (uint64_t)den);
  out->num = num / (int64_t)g;
  out->den = den / (int64_t)g;
  return COS_OK;
}

CosStatus cos_rat_int(int64_t v, CosRat *out) { return cos_rat_make(v, 1, out); }

CosStatus cos_rat_neg(CosRat a, CosRat *out) {
  if (!cos_rat_valid(a)) return COS_E_INVALID;
  out->num = -a.num;
  out->den = a.den;
  return COS_OK;
}

CosStatus cos_rat_abs(CosRat a, CosRat *out) {
  if (!cos_rat_valid(a)) return COS_E_INVALID;
  out->num = a.num < 0 ? -a.num : a.num;
  out->den = a.den;
  return COS_OK;
}

CosStatus cos_rat_add(CosRat a, CosRat b, CosRat *out) {
  if (!cos_rat_valid(a) || !cos_rat_valid(b)) return COS_E_INVALID;
  const uint64_t g = gcd_u((uint64_t)a.den, (uint64_t)b.den);
  const uint64_t ad = (uint64_t)a.den / g, bd = (uint64_t)b.den / g;
  const U128 p = mul_u128(mag(a.num), bd);
  const U128 q = mul_u128(mag(b.num), ad);
  U128 t;
  bool neg;
  if ((a.num < 0) == (b.num < 0)) {
    t = add128(p, q);
    neg = a.num < 0;
  } else {
    const int c = cmp128(p, q);
    if (c == 0) {
      out->num = 0;
      out->den = 1;
      return COS_OK;
    }
    t = c > 0 ? sub128(p, q) : sub128(q, p);
    neg = c > 0 ? a.num < 0 : b.num < 0;
  }
  if (t.hi == 0u && t.lo == 0u) {
    out->num = 0;
    out->den = 1;
    return COS_OK;
  }
  uint64_t rem;
  (void)divmod128_u64(t, g, &rem);
  const uint64_t g2 = gcd_u(rem, g);
  const U128 reduced = divmod128_u64(t, g2, &rem);
  if (rem != 0u || reduced.hi != 0u || reduced.lo > (uint64_t)RMAX) return COS_E_RANGE;
  const U128 den = mul_u128(ad, (uint64_t)b.den / g2);
  if (den.hi != 0u || den.lo > (uint64_t)RMAX) return COS_E_RANGE;
  out->num = neg ? -(int64_t)reduced.lo : (int64_t)reduced.lo;
  out->den = (int64_t)den.lo;
  return COS_OK;
}

CosStatus cos_rat_sub(CosRat a, CosRat b, CosRat *out) {
  if (!cos_rat_valid(b)) return COS_E_INVALID;
  b.num = -b.num;
  return cos_rat_add(a, b, out);
}

CosStatus cos_rat_mul(CosRat a, CosRat b, CosRat *out) {
  if (!cos_rat_valid(a) || !cos_rat_valid(b)) return COS_E_INVALID;
  if (a.num == 0 || b.num == 0) {
    out->num = 0;
    out->den = 1;
    return COS_OK;
  }
  const int64_t g1 = (int64_t)gcd_u(mag(a.num), (uint64_t)b.den), g2 = (int64_t)gcd_u(mag(b.num), (uint64_t)a.den);
  int64_t num, den;
  if (mul_ovf(a.num / g1, b.num / g2, &num) || mul_ovf(a.den / g2, b.den / g1, &den)) return COS_E_RANGE;
  out->num = num;
  out->den = den;
  return COS_OK;
}

CosStatus cos_rat_div(CosRat a, CosRat b, CosRat *out) {
  if (!cos_rat_valid(a) || !cos_rat_valid(b)) return COS_E_INVALID;
  if (b.num == 0) return COS_E_INVALID; /* division by zero is not a number: Forbidden Zone */
  CosRat inv = {b.den, b.num};
  if (b.num < 0) {
    inv.num = -b.den;
    inv.den = -b.num;
  }
  return cos_rat_mul(a, inv, out);
}

CosStatus cos_rat_pow(CosRat a, int32_t e, CosRat *out) {
  if (!cos_rat_valid(a)) return COS_E_INVALID;
  if (a.num == 0 && e <= 0) return COS_E_INVALID; /* 0^0 is undefined here, and 0 to a negative power is a division by zero */
  CosRat base = a;
  int64_t k = e;
  if (k < 0) {
    k = -k;
    CosRat one = {1, 1}, inv;
    const CosStatus st = cos_rat_div(one, a, &inv);
    if (st != COS_OK) return st;
    base = inv;
  }
  CosRat acc = {1, 1};
  while (k > 0) {
    if ((k & 1) != 0) {
      const CosStatus st = cos_rat_mul(acc, base, &acc);
      if (st != COS_OK) return st;
    }
    k >>= 1;
    if (k > 0) {
      const CosStatus st = cos_rat_mul(base, base, &base);
      if (st != COS_OK) return st; /* even if the answer might still fit, the square on the way did not: refused, never rounded */
    }
  }
  *out = acc;
  return COS_OK;
}

int cos_rat_sign(CosRat a) { return a.num > 0 ? 1 : (a.num < 0 ? -1 : 0); }
bool cos_rat_is_int(CosRat a) { return a.den == 1; }

int cos_rat_cmp(CosRat a, CosRat b) {
  const int sa = cos_rat_sign(a), sb = cos_rat_sign(b);
  if (sa != sb) return sa < sb ? -1 : 1;
  if (sa == 0) return 0;
  uint64_t h1, l1, h2, l2;
  mul128(mag(a.num), (uint64_t)b.den, &h1, &l1); /* |a| < |b|  <=>  |a.num| * b.den < |b.num| * a.den, both products exact in 128 bits */
  mul128(mag(b.num), (uint64_t)a.den, &h2, &l2);
  int c = 0;
  if (h1 != h2) c = h1 < h2 ? -1 : 1;
  else if (l1 != l2) c = l1 < l2 ? -1 : 1;
  return sa > 0 ? c : -c;
}

/* ---------------------------------------------------------------- reading a decimal number */

static bool is_digit(uint8_t c) { return c >= '0' && c <= '9'; }

CosStatus cos_rat_parse(const uint8_t *s, size_t n, CosRat *out) {
  if (n > 0u && s == NULL) return COS_E_INVALID;
  size_t i = 0;
  bool neg = false;
  if (i < n && s[i] == '-') {
    neg = true;
    i++;
  }
  if (i >= n) return COS_E_INVALID;
  uint64_t m = 0;        /* the significant digits so far (leading zeros do not count) */
  uint64_t pending = 0;  /* zeros seen since the last nonzero digit: trailing so far, applied if another nonzero digit follows */
  bool over = false;     /* more than 64 bits of significant digits */
  int64_t scale = 0;     /* value = m * 10^(scale + pending) */
#define PUSH(d)                                                                                            \
  do {                                                                                                     \
    if ((d) == 0u) {                                                                                       \
      if (m != 0u) pending++;                                                                              \
    } else {                                                                                               \
      while (pending > 0u && !over) {                                                                      \
        if (m > UINT64_MAX / 10u) over = true;                                                             \
        else m *= 10u;                                                                                     \
        pending--;                                                                                         \
      }                                                                                                    \
      pending = 0;                                                                                         \
      if (!over) {                                                                                         \
        if (m > (UINT64_MAX - (d)) / 10u) over = true;                                                     \
        else m = m * 10u + (d);                                                                            \
      }                                                                                                    \
    }                                                                                                      \
  } while (0)
  if (s[i] == '0') {
    i++;
    if (i < n && is_digit(s[i])) return COS_E_INVALID; /* no leading zeros */
  } else if (s[i] >= '1' && s[i] <= '9') {
    while (i < n && is_digit(s[i])) {
      const uint64_t d = (uint64_t)(s[i] - '0');
      PUSH(d);
      i++;
    }
  } else {
    return COS_E_INVALID;
  }
  if (i < n && s[i] == '.') {
    i++;
    const size_t f0 = i;
    while (i < n && is_digit(s[i])) {
      const uint64_t d = (uint64_t)(s[i] - '0');
      PUSH(d);
      scale--;
      i++;
    }
    if (i == f0) return COS_E_INVALID; /* a point needs digits after it */
  }
  if (i < n && (s[i] == 'e' || s[i] == 'E')) {
    i++;
    bool eneg = false;
    if (i < n && (s[i] == '+' || s[i] == '-')) {
      eneg = s[i] == '-';
      i++;
    }
    const size_t e0 = i;
    int64_t e = 0;
    while (i < n && is_digit(s[i])) {
      if (e < 1000000) e = e * 10 + (s[i] - '0'); /* saturates: anything this large is out of range anyway */
      i++;
    }
    if (i == e0) return COS_E_INVALID;
    scale += eneg ? -e : e;
  }
#undef PUSH
  if (i != n) return COS_E_INVALID;
  if (m == 0u) {
    out->num = 0;
    out->den = 1;
    return COS_OK;
  }
  if (over) return COS_E_RANGE;
  int64_t ex = scale + (int64_t)pending; /* value = m * 10^ex; pending is the trailing zeros */
  uint64_t num = m, den = 1;
  if (ex >= 0) {
    for (int64_t k = 0; k < ex; ++k) {
      if (num > (uint64_t)RMAX / 10u) return COS_E_RANGE;
      num *= 10u;
    }
  } else {
    /* divide by 10^k = 2^k 5^k: cancel what m has of each factor first, so a value that fits is never lost to the size of 10^k */
    int64_t k2 = -ex, k5 = -ex;
    while (k2 > 0 && (num & 1u) == 0u) { num >>= 1; k2--; }
    while (k5 > 0 && num % 5u == 0u) { num /= 5u; k5--; }
    for (int64_t k = 0; k < k2; ++k) {
      if (den > (uint64_t)RMAX / 2u) return COS_E_RANGE;
      den *= 2u;
    }
    for (int64_t k = 0; k < k5; ++k) {
      if (den > (uint64_t)RMAX / 5u) return COS_E_RANGE;
      den *= 5u;
    }
  }
  if (num > (uint64_t)RMAX || den > (uint64_t)RMAX) return COS_E_RANGE;
  out->num = neg ? -(int64_t)num : (int64_t)num;
  out->den = (int64_t)den;
  return COS_OK;
}

/* ---------------------------------------------------------------- writing */

static size_t put_u64(uint64_t v, char *out, size_t cap, size_t at) {
  char tmp[20];
  size_t k = 0;
  do {
    tmp[k++] = (char)('0' + (unsigned)(v % 10u));
    v /= 10u;
  } while (v != 0u);
  if (at + k > cap) return 0;
  while (k > 0u) out[at++] = tmp[--k];
  return at;
}

size_t cos_rat_format(CosRat a, char *out, size_t cap) {
  if (cap == 0u) return 0;
  out[0] = '\0';
  if (!cos_rat_valid(a)) return 0;
  size_t at = 0;
  if (a.num < 0) {
    if (cap < 2u) return 0;
    out[at++] = '-';
  }
  at = put_u64(mag(a.num), out, cap - 1u, at);
  if (at == 0u) { out[0] = '\0'; return 0; }
  if (a.den != 1) {
    if (at + 1u > cap - 1u) { out[0] = '\0'; return 0; }
    out[at++] = '/';
    at = put_u64((uint64_t)a.den, out, cap - 1u, at);
    if (at == 0u) { out[0] = '\0'; return 0; }
  }
  out[at] = '\0';
  return at;
}

/* one more decimal digit of r/den: r*10 can need 68 bits, so it is held in two words */
static unsigned next_digit(uint64_t *r, uint64_t den) {
  uint64_t hi, lo;
  mul128(*r, 10u, &hi, &lo);
  unsigned d = 0;
  while (hi != 0u || lo >= den) {
    if (lo < den) hi--;
    lo -= den;
    d++;
  }
  *r = lo;
  return d;
}

size_t cos_rat_decimal(CosRat a, uint32_t places, char *out, size_t cap, bool *exact) {
  if (exact != NULL) *exact = false;
  if (cap == 0u) return 0;
  out[0] = '\0';
  if (!cos_rat_valid(a) || places > 40u) return 0;
  const uint64_t den = (uint64_t)a.den;
  uint64_t r = mag(a.num) % den;
  const uint64_t ip = mag(a.num) / den;
  char digits[40];
  uint32_t nd = 0;
  bool any_nonzero = ip != 0u;
  while (nd < places && r != 0u) {
    const unsigned d = next_digit(&r, den);
    digits[nd++] = (char)('0' + d);
    if (d != 0u) any_nonzero = true;
  }
  size_t at = 0;
  if (a.num < 0 && any_nonzero) {
    if (cap < 2u) return 0;
    out[at++] = '-';
  }
  at = put_u64(ip, out, cap - 1u, at);
  if (at == 0u) { out[0] = '\0'; return 0; }
  if (nd > 0u) {
    if (at + 1u + nd > cap - 1u) { out[0] = '\0'; return 0; }
    out[at++] = '.';
    for (uint32_t k = 0; k < nd; ++k) out[at++] = digits[k];
  }
  out[at] = '\0';
  if (exact != NULL) *exact = (r == 0u);
  return at;
}
