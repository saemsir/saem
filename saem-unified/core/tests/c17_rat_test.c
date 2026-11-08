/* Exact rationals. Hand-written tables for the corners (overflow on every operation, INT64_MIN, a result that fits although the obvious way of
 * computing it overflows, the number grammar, text output), plus properties on random operands. The broad comparison with an arbitrary-precision
 * oracle (Python's Fraction, thousands of random operations per class) is in tools/check_c17.py. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "c17_check.h"
#include "cos_rat.h"

#define MAXV INT64_MAX

static CosRat R(int64_t n, int64_t d) {
  CosRat r = {0, 1};
  (void)cos_rat_make(n, d, &r);
  return r;
}
static int eq(CosRat a, int64_t n, int64_t d) { return a.num == n && a.den == d; }
static const CosRat SENTINEL = {123456789, 987654321}; /* a valid-looking value to prove an error leaves *out alone */

static CosStatus P(const char *t, CosRat *out) { return cos_rat_parse((const uint8_t *)t, strlen(t), out); }

static void test_make(CosSuite *s) {
  CosRat r;
  CK(s, cos_rat_make(6, 8, &r) == COS_OK && eq(r, 3, 4));
  CK(s, cos_rat_make(-6, 8, &r) == COS_OK && eq(r, -3, 4));
  CK(s, cos_rat_make(6, -8, &r) == COS_OK && eq(r, -3, 4));
  CK(s, cos_rat_make(-6, -8, &r) == COS_OK && eq(r, 3, 4));
  CK(s, cos_rat_make(0, 5, &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_make(0, -5, &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_make(MAXV, MAXV, &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_make(MAXV, 1, &r) == COS_OK && eq(r, MAXV, 1));
  CK(s, cos_rat_make(-MAXV, 1, &r) == COS_OK && eq(r, -MAXV, 1));
  CK(s, cos_rat_make(MAXV, -1, &r) == COS_OK && eq(r, -MAXV, 1));
  CK(s, cos_rat_make(1, MAXV, &r) == COS_OK && eq(r, 1, MAXV));
  CK(s, cos_rat_make(1, -MAXV, &r) == COS_OK && eq(r, -1, MAXV));
  CK(s, cos_rat_int(-42, &r) == COS_OK && eq(r, -42, 1));
  /* the errors leave the output alone */
  r = SENTINEL;
  CK(s, cos_rat_make(5, 0, &r) == COS_E_INVALID && eq(r, 123456789, 987654321));
  CK(s, cos_rat_make(0, 0, &r) == COS_E_INVALID);
  CK(s, cos_rat_make(INT64_MIN, 1, &r) == COS_E_RANGE && eq(r, 123456789, 987654321));
  CK(s, cos_rat_make(1, INT64_MIN, &r) == COS_E_RANGE);
  CK(s, cos_rat_make(INT64_MIN, INT64_MIN, &r) == COS_E_RANGE);
  CK(s, cos_rat_int(INT64_MIN, &r) == COS_E_RANGE);
  /* validity */
  CosRat bad[] = {{1, 0}, {0, 2}, {2, 4}, {1, -1}, {INT64_MIN, 1}, {5, INT64_MIN}, {0, 0}, {-3, -4}, {6, 9}, {0, -1}};
  int invalid = 0;
  for (size_t i = 0; i < sizeof bad / sizeof bad[0]; ++i) invalid += !cos_rat_valid(bad[i]);
  CK(s, invalid == (int)(sizeof bad / sizeof bad[0]));
  CK(s, cos_rat_valid(R(0, 1)) && cos_rat_valid(R(7, 1)) && cos_rat_valid(R(-3, 4)) && cos_rat_valid(R(MAXV, MAXV - 1)));
  /* every operation refuses an invalid operand instead of computing with it */
  CosRat ok = R(1, 2);
  r = SENTINEL;
  CK(s, cos_rat_add(bad[2], ok, &r) == COS_E_INVALID && cos_rat_add(ok, bad[0], &r) == COS_E_INVALID);
  CK(s, cos_rat_sub(bad[2], ok, &r) == COS_E_INVALID && cos_rat_mul(bad[3], ok, &r) == COS_E_INVALID);
  CK(s, cos_rat_div(ok, bad[1], &r) == COS_E_INVALID && cos_rat_neg(bad[2], &r) == COS_E_INVALID && cos_rat_abs(bad[2], &r) == COS_E_INVALID);
  CK(s, cos_rat_pow(bad[2], 2, &r) == COS_E_INVALID && eq(r, 123456789, 987654321));
}

static void test_arithmetic(CosSuite *s) {
  CosRat r;
  CK(s, cos_rat_add(R(1, 2), R(1, 3), &r) == COS_OK && eq(r, 5, 6));
  CK(s, cos_rat_add(R(1, 6), R(1, 3), &r) == COS_OK && eq(r, 1, 2));    /* common factor in the denominators */
  CK(s, cos_rat_add(R(1, 2), R(-1, 2), &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_add(R(-1, 6), R(-1, 3), &r) == COS_OK && eq(r, -1, 2));
  CK(s, cos_rat_add(R(5, 1), R(0, 1), &r) == COS_OK && eq(r, 5, 1));
  CK(s, cos_rat_sub(R(1, 2), R(1, 3), &r) == COS_OK && eq(r, 1, 6));
  CK(s, cos_rat_sub(R(1, 3), R(1, 2), &r) == COS_OK && eq(r, -1, 6));
  CK(s, cos_rat_sub(R(7, 9), R(7, 9), &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_mul(R(2, 3), R(3, 4), &r) == COS_OK && eq(r, 1, 2));
  CK(s, cos_rat_mul(R(-2, 3), R(3, 4), &r) == COS_OK && eq(r, -1, 2));
  CK(s, cos_rat_mul(R(-2, 3), R(-3, 4), &r) == COS_OK && eq(r, 1, 2));
  CK(s, cos_rat_mul(R(0, 1), R(MAXV, 3), &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_div(R(2, 3), R(4, 9), &r) == COS_OK && eq(r, 3, 2));
  CK(s, cos_rat_div(R(-2, 3), R(-4, 9), &r) == COS_OK && eq(r, 3, 2));
  CK(s, cos_rat_div(R(5, 1), R(-1, 3), &r) == COS_OK && eq(r, -15, 1));
  CK(s, cos_rat_div(R(0, 1), R(7, 3), &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_neg(R(3, 4), &r) == COS_OK && eq(r, -3, 4) && cos_rat_neg(R(-3, 4), &r) == COS_OK && eq(r, 3, 4));
  CK(s, cos_rat_neg(R(-MAXV, 1), &r) == COS_OK && eq(r, MAXV, 1)); /* negation is total because INT64_MIN is not a value */
  CK(s, cos_rat_abs(R(-3, 4), &r) == COS_OK && eq(r, 3, 4));
  /* division by zero is not a number */
  r = SENTINEL;
  CK(s, cos_rat_div(R(1, 2), R(0, 1), &r) == COS_E_INVALID && eq(r, 123456789, 987654321));
  CK(s, cos_rat_div(R(0, 1), R(0, 1), &r) == COS_E_INVALID);
  /* overflow is refused, on every operation, in both signs, and leaves the output alone */
  CK(s, cos_rat_add(R(MAXV, 1), R(1, 1), &r) == COS_E_RANGE && eq(r, 123456789, 987654321));
  CK(s, cos_rat_add(R(-MAXV, 1), R(-1, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_sub(R(MAXV, 1), R(-1, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_sub(R(-MAXV, 1), R(1, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_mul(R(MAXV, 1), R(2, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_mul(R(-MAXV, 1), R(2, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_mul(R(3037000500, 1), R(3037000500, 1), &r) == COS_E_RANGE);  /* just past 2^63 */
  CK(s, cos_rat_mul(R(3037000499, 1), R(3037000499, 1), &r) == COS_OK && eq(r, 9223372030926249001LL, 1)); /* just inside */
  CK(s, cos_rat_mul(R(1, MAXV), R(1, 2), &r) == COS_E_RANGE);                  /* the denominator would not fit */
  CK(s, cos_rat_add(R(1, MAXV), R(1, MAXV - 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_div(R(1, MAXV), R(2, 1), &r) == COS_E_RANGE);
  CK(s, cos_rat_div(R(1, 1), R(1, MAXV), &r) == COS_OK && eq(r, MAXV, 1));
  CK(s, cos_rat_div(R(MAXV, 1), R(-1, 1), &r) == COS_OK && eq(r, -MAXV, 1));
  CK(s, cos_rat_add(R(MAXV, 1), R(-MAXV, 1), &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_add(R(MAXV - 1, 1), R(1, 1), &r) == COS_OK && eq(r, MAXV, 1));
  /* Wide intermediates preserve sums whose unreduced numerator exceeds 64 bits but whose exact reduced result fits. */
  r = SENTINEL;
  CK(s, cos_rat_add(R(MAXV, (int64_t)1 << 62), R(3, (int64_t)1 << 62), &r) == COS_OK && eq(r, 4611686018427387905LL, 2305843009213693952LL));
  CK(s, cos_rat_add(R(3, (int64_t)1 << 62), R(MAXV, (int64_t)1 << 62), &r) == COS_OK && eq(r, 4611686018427387905LL, 2305843009213693952LL));
  CK(s, cos_rat_add(R(-MAXV, (int64_t)1 << 62), R(-3, (int64_t)1 << 62), &r) == COS_OK && eq(r, -4611686018427387905LL, 2305843009213693952LL));
  CK(s, cos_rat_add(R(MAXV, (int64_t)1 << 62), R(1, (int64_t)1 << 62), &r) == COS_OK && eq(r, 2, 1));
  CK(s, cos_rat_add(R(1, (int64_t)1 << 62), R(MAXV, (int64_t)1 << 62), &r) == COS_OK && eq(r, 2, 1));
  CK(s, cos_rat_sub(R(MAXV, (int64_t)1 << 62), R(-3, (int64_t)1 << 62), &r) == COS_OK && eq(r, 4611686018427387905LL, 2305843009213693952LL));
  CK(s, cos_rat_add(R(MAXV - 2, (int64_t)1 << 62), R(1, (int64_t)1 << 62), &r) == COS_OK && eq(r, 4611686018427387903LL, 2305843009213693952LL)); /* (2^63-2)/2^62 = (2^62-1)/2^61 */
  r = SENTINEL;
  CK(s, cos_rat_add(R(MAXV, 2), R(MAXV - 1, 2), &r) == COS_E_RANGE && eq(r, 123456789, 987654321));
  CK(s, cos_rat_sub(R(MAXV, 2), R(-(MAXV - 1), 2), &r) == COS_E_RANGE && eq(r, 123456789, 987654321));
  /* an answer that fits is not lost to an overflow on the way (reduce before multiplying) */
  CK(s, cos_rat_mul(R(3, MAXV), R(MAXV, 3), &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_mul(R(MAXV, 7), R(7, MAXV), &r) == COS_OK && eq(r, 1, 1));
  const int64_t two62 = (int64_t)1 << 62;
  CK(s, cos_rat_add(R(1, two62), R(1, two62), &r) == COS_OK && eq(r, 1, two62 / 2));
  CK(s, cos_rat_sub(R(1, two62), R(1, two62), &r) == COS_OK && eq(r, 0, 1));
  r = R(MAXV, two62);
  CK(s, cos_rat_add(r, R(3, two62), &r) == COS_OK && eq(r, 4611686018427387905LL, 2305843009213693952LL));
  r = R(MAXV, two62);
  CK(s, cos_rat_sub(r, R(-3, two62), &r) == COS_OK && eq(r, 4611686018427387905LL, 2305843009213693952LL));
  CK(s, cos_rat_div(R(MAXV, 5), R(MAXV, 3), &r) == COS_OK && eq(r, 3, 5));
  CK(s, cos_rat_add(R(MAXV, 2), R(-MAXV, 2), &r) == COS_OK && eq(r, 0, 1));
  /* operations compose: (a + b) - b == a, (a * b) / b == a, for operands whose products fit */
  int fails = 0;
  for (int64_t a = -20; a <= 20; ++a)
    for (int64_t b = 1; b <= 20; ++b)
      for (int64_t c = -9; c <= 9; ++c)
        for (int64_t d = 1; d <= 9; ++d) {
          CosRat x = R(a, b), y = R(c, d), t, u;
          if (cos_rat_add(x, y, &t) != COS_OK || cos_rat_sub(t, y, &u) != COS_OK || !eq(u, x.num, x.den)) fails++;
          if (c != 0) {
            if (cos_rat_mul(x, y, &t) != COS_OK || cos_rat_div(t, y, &u) != COS_OK || !eq(u, x.num, x.den)) fails++;
          }
          if (cos_rat_add(x, y, &t) != COS_OK || cos_rat_add(y, x, &u) != COS_OK || !eq(t, u.num, u.den)) fails++; /* commutative */
          if (!cos_rat_valid(t)) fails++;
        }
  CK(s, fails == 0);
}

static void test_pow(CosSuite *s) {
  CosRat r;
  CK(s, cos_rat_pow(R(2, 1), 10, &r) == COS_OK && eq(r, 1024, 1));
  CK(s, cos_rat_pow(R(2, 3), 3, &r) == COS_OK && eq(r, 8, 27));
  CK(s, cos_rat_pow(R(-2, 1), 3, &r) == COS_OK && eq(r, -8, 1));
  CK(s, cos_rat_pow(R(-2, 1), 4, &r) == COS_OK && eq(r, 16, 1));
  CK(s, cos_rat_pow(R(2, 1), -3, &r) == COS_OK && eq(r, 1, 8));
  CK(s, cos_rat_pow(R(-2, 3), -3, &r) == COS_OK && eq(r, -27, 8));
  CK(s, cos_rat_pow(R(5, 7), 0, &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_pow(R(0, 1), 5, &r) == COS_OK && eq(r, 0, 1));
  CK(s, cos_rat_pow(R(1, 1), INT32_MAX, &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_pow(R(-1, 1), INT32_MAX, &r) == COS_OK && eq(r, -1, 1));
  CK(s, cos_rat_pow(R(-1, 1), INT32_MIN, &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_pow(R(1, 1), INT32_MIN, &r) == COS_OK && eq(r, 1, 1));
  CK(s, cos_rat_pow(R(2, 1), 62, &r) == COS_OK && eq(r, (int64_t)1 << 62, 1));
  CK(s, cos_rat_pow(R(-2, 1), 63, &r) == COS_E_RANGE);          /* -2^63 is INT64_MIN: not a value */
  CK(s, cos_rat_pow(R(2, 1), 63, &r) == COS_E_RANGE);
  CK(s, cos_rat_pow(R(2, 1), -62, &r) == COS_OK && eq(r, 1, (int64_t)1 << 62));
  CK(s, cos_rat_pow(R(2, 1), -63, &r) == COS_E_RANGE);
  CK(s, cos_rat_pow(R(3, 1), 39, &r) == COS_OK && eq(r, 4052555153018976267LL, 1)); /* 3^39 */
  CK(s, cos_rat_pow(R(3, 1), 40, &r) == COS_E_RANGE);
  CK(s, cos_rat_pow(R(10, 1), 18, &r) == COS_OK && eq(r, 1000000000000000000LL, 1));
  CK(s, cos_rat_pow(R(10, 1), 19, &r) == COS_E_RANGE);
  CK(s, cos_rat_pow(R(7, 3), INT32_MAX, &r) == COS_E_RANGE);
  r = SENTINEL;
  CK(s, cos_rat_pow(R(0, 1), 0, &r) == COS_E_INVALID && eq(r, 123456789, 987654321)); /* 0^0 is not defined here */
  CK(s, cos_rat_pow(R(0, 1), -1, &r) == COS_E_INVALID);
  CK(s, cos_rat_pow(R(0, 1), INT32_MIN, &r) == COS_E_INVALID);
}

static void test_cmp(CosSuite *s) {
  CK(s, cos_rat_cmp(R(1, 2), R(1, 3)) == 1 && cos_rat_cmp(R(1, 3), R(1, 2)) == -1 && cos_rat_cmp(R(1, 2), R(1, 2)) == 0);
  CK(s, cos_rat_cmp(R(-1, 2), R(-1, 3)) == -1 && cos_rat_cmp(R(-1, 3), R(-1, 2)) == 1);
  CK(s, cos_rat_cmp(R(-1, 2), R(1, 3)) == -1 && cos_rat_cmp(R(1, 3), R(-1, 2)) == 1);
  CK(s, cos_rat_cmp(R(0, 1), R(1, MAXV)) == -1 && cos_rat_cmp(R(0, 1), R(-1, MAXV)) == 1 && cos_rat_cmp(R(0, 1), R(0, 1)) == 0);
  /* products of 63-bit numbers need 126 bits: the comparison is still exact */
  const CosRat a = R(MAXV - 1, MAXV), b = R(MAXV - 2, MAXV - 1); /* 1 - 1/M  versus  1 - 1/(M-1) */
  CK(s, cos_rat_cmp(a, b) == 1 && cos_rat_cmp(b, a) == -1);
  CK(s, cos_rat_cmp(R(-(MAXV - 1), MAXV), R(-(MAXV - 2), MAXV - 1)) == -1);
  CK(s, cos_rat_cmp(R(MAXV, 1), R(MAXV - 1, 1)) == 1 && cos_rat_cmp(R(-MAXV, 1), R(MAXV, 1)) == -1);
  CK(s, cos_rat_cmp(R(1, MAXV), R(1, MAXV - 1)) == -1);
  CK(s, cos_rat_cmp(R(MAXV, MAXV - 1), R(MAXV - 1, MAXV - 2)) == -1); /* 1 + 1/(M-1)  <  1 + 1/(M-2) */
  /* for operands whose difference fits, cmp is the sign of the difference; it is antisymmetric and transitive */
  int fails = 0;
  for (int64_t n1 = -6; n1 <= 6; ++n1)
    for (int64_t d1 = 1; d1 <= 6; ++d1)
      for (int64_t n2 = -6; n2 <= 6; ++n2)
        for (int64_t d2 = 1; d2 <= 6; ++d2) {
          const CosRat x = R(n1, d1), y = R(n2, d2);
          CosRat t;
          (void)cos_rat_sub(x, y, &t);
          if (cos_rat_cmp(x, y) != cos_rat_sign(t) || cos_rat_cmp(x, y) != -cos_rat_cmp(y, x)) fails++;
          for (int64_t n3 = -2; n3 <= 2; ++n3) {
            const CosRat z = R(n3, 3);
            if (cos_rat_cmp(x, y) <= 0 && cos_rat_cmp(y, z) <= 0 && cos_rat_cmp(x, z) > 0) fails++;
          }
        }
  CK(s, fails == 0);
  CK(s, cos_rat_sign(R(-3, 4)) == -1 && cos_rat_sign(R(0, 1)) == 0 && cos_rat_sign(R(3, 4)) == 1);
  CK(s, cos_rat_is_int(R(8, 2)) && !cos_rat_is_int(R(7, 2)) && cos_rat_is_int(R(0, 1)));
}

static void test_parse(CosSuite *s) {
  struct { const char *t; int64_t n, d; } ok[] = {
      {"0", 0, 1}, {"-0", 0, 1}, {"7", 7, 1}, {"-7", -7, 1}, {"1.5", 3, 2}, {"0.1", 1, 10}, {"-0.25", -1, 4}, {"1e3", 1000, 1}, {"1E3", 1000, 1},
      {"1e+3", 1000, 1}, {"1e-3", 1, 1000}, {"12.5e1", 125, 1}, {"12.5e-1", 5, 4}, {"0.000001", 1, 1000000}, {"100", 100, 1}, {"1.50", 3, 2},
      {"1.0", 1, 1}, {"0.0", 0, 1}, {"0e0", 0, 1}, {"-0.0e5", 0, 1}, {"0e999999", 0, 1}, {"0.0e-99999999999999999999", 0, 1},
      {"9223372036854775807", MAXV, 1}, {"-9223372036854775807", -MAXV, 1}, {"1e18", 1000000000000000000LL, 1},
      {"9223372036854775815e-1", 1844674407370955163LL, 2},          /* the digits need 64 bits, the value only 63 */
      {"9223372036854775808e-1", 4611686018427387904LL, 5},          /* 2^63/10 = 2^62/5: reduces, so it fits although the digits alone do not */
      {"0.5000000000000000000000000", 1, 2},                          /* trailing zeros do not count as digits */
      {"1.000000000000000000000000000000", 1, 1},
      {"5e-19", 1, 2000000000000000000LL}, {"1e-18", 1, 1000000000000000000LL}, {"2.5e0", 5, 2}, {"-1e-1", -1, 10},
      {"123456789012345678", 123456789012345678LL, 1}, {"0.00000000000000001", 1, 100000000000000000LL}, {"1.2345678901234567890", 1234567890123456789LL, 1000000000000000000LL},
  };
  int bad = 0;
  for (size_t i = 0; i < sizeof ok / sizeof ok[0]; ++i) {
    CosRat r = SENTINEL;
    const CosStatus st = P(ok[i].t, &r);
    if (st != COS_OK || !eq(r, ok[i].n, ok[i].d) || !cos_rat_valid(r)) {
      bad++;
      fprintf(stderr, "parse '%s': status %d value %lld/%lld\n", ok[i].t, (int)st, (long long)r.num, (long long)r.den);
    }
  }
  CK(s, bad == 0);
  /* not representable exactly: refused as RANGE, output untouched */
  const char *range[] = {"9223372036854775808", "-9223372036854775808", "18446744073709551615", "18446744073709551616", "1e19", "1e1000", "-1e19",
                         "1e99999999999999999999", "1e-99999999999999999999", "1e-19", "123456789012345678901", "1.00000000000000000000000000001",
                         "0.00000000000000000001", "99999999999999999999e-20", "3e-19"};
  int rbad = 0;
  for (size_t i = 0; i < sizeof range / sizeof range[0]; ++i) {
    CosRat r = SENTINEL;
    if (P(range[i], &r) != COS_E_RANGE || !eq(r, 123456789, 987654321)) { rbad++; fprintf(stderr, "range '%s'\n", range[i]); }
  }
  CK(s, rbad == 0);
  /* not a JSON number at all: INVALID */
  const char *inval[] = {"", "-", "+1", "01", "00", "-01", ".5", "5.", "1.e3", "1e", "1e+", "1e-", "1.2.3", "1e2e3", " 1", "1 ", "0x10", "NaN",
                         "Infinity", "-Infinity", "1_000", "1,5", "--1", "-+1", "1ee3", "e3", "-e3", "0.e1", "1.5.0", "\xD9\xA1", "1\xD9\xA1", "٣", "1/2",
                         "0.", "-.5", "1e5.0", "٫", "--", "-a", "1a", "0b1", "1f", "1d"};
  int ibad = 0;
  for (size_t i = 0; i < sizeof inval / sizeof inval[0]; ++i) {
    CosRat r = SENTINEL;
    if (P(inval[i], &r) != COS_E_INVALID || !eq(r, 123456789, 987654321)) { ibad++; fprintf(stderr, "invalid '%s'\n", inval[i]); }
  }
  CK(s, ibad == 0);
  CosRat r = SENTINEL;
  CK(s, cos_rat_parse((const uint8_t *)"1\0" "2", 3, &r) == COS_E_INVALID);       /* an embedded NUL is not a digit */
  CK(s, cos_rat_parse(NULL, 0, &r) == COS_E_INVALID && cos_rat_parse(NULL, 3, &r) == COS_E_INVALID);
  CK(s, cos_rat_parse((const uint8_t *)"12345", 2, &r) == COS_OK && eq(r, 12, 1)); /* length is honoured, not the terminator */
  /* every prefix of a valid number is parsed on its own: nothing reads past n */
  const uint8_t *src = (const uint8_t *)"-12.345e-6";
  int pfx = 0;
  for (size_t n = 0; n <= 10; ++n) {
    CosRat q;
    const CosStatus st = cos_rat_parse(src, n, &q);
    pfx += (st == COS_OK || st == COS_E_INVALID || st == COS_E_RANGE);
  }
  CK(s, pfx == 11);
}

static void test_text(CosSuite *s) {
  char buf[64];
  CK(s, cos_rat_format(R(3, 4), buf, sizeof buf) == 3 && strcmp(buf, "3/4") == 0);
  CK(s, cos_rat_format(R(-3, 4), buf, sizeof buf) == 4 && strcmp(buf, "-3/4") == 0);
  CK(s, cos_rat_format(R(5, 1), buf, sizeof buf) == 1 && strcmp(buf, "5") == 0);
  CK(s, cos_rat_format(R(-5, 1), buf, sizeof buf) == 2 && strcmp(buf, "-5") == 0);
  CK(s, cos_rat_format(R(0, 1), buf, sizeof buf) == 1 && strcmp(buf, "0") == 0);
  CK(s, cos_rat_format(R(MAXV, 1), buf, sizeof buf) == 19 && strcmp(buf, "9223372036854775807") == 0);
  CK(s, cos_rat_format(R(-MAXV, MAXV - 1), buf, sizeof buf) == 40 && strcmp(buf, "-9223372036854775807/9223372036854775806") == 0);
  /* a buffer that is too small gives 0 and an empty string, never a cut-off number */
  CK(s, cos_rat_format(R(3, 4), buf, 0) == 0);
  buf[0] = 'x';
  CK(s, cos_rat_format(R(3, 4), buf, 1) == 0 && buf[0] == '\0');
  CK(s, cos_rat_format(R(3, 4), buf, 3) == 0 && buf[0] == '\0');
  CK(s, cos_rat_format(R(3, 4), buf, 4) == 3 && strcmp(buf, "3/4") == 0);
  CK(s, cos_rat_format(R(-3, 4), buf, 4) == 0);
  CK(s, cos_rat_format(R(-3, 4), buf, 5) == 4);
  CK(s, cos_rat_format((CosRat){2, 4}, buf, sizeof buf) == 0); /* an invalid value is not printed */
  bool ex;
  CK(s, cos_rat_decimal(R(1, 3), 5, buf, sizeof buf, &ex) == 7 && strcmp(buf, "0.33333") == 0 && !ex);
  CK(s, cos_rat_decimal(R(1, 4), 5, buf, sizeof buf, &ex) == 4 && strcmp(buf, "0.25") == 0 && ex);
  CK(s, cos_rat_decimal(R(-1, 4), 5, buf, sizeof buf, &ex) == 5 && strcmp(buf, "-0.25") == 0 && ex);
  CK(s, cos_rat_decimal(R(-1, 3), 0, buf, sizeof buf, &ex) == 1 && strcmp(buf, "0") == 0 && !ex);   /* no "-0" */
  CK(s, cos_rat_decimal(R(-1, 3), 3, buf, sizeof buf, &ex) == 6 && strcmp(buf, "-0.333") == 0 && !ex);
  CK(s, cos_rat_decimal(R(-1, 1000), 2, buf, sizeof buf, &ex) == 4 && strcmp(buf, "0.00") == 0 && !ex);
  CK(s, cos_rat_decimal(R(7, 1), 4, buf, sizeof buf, &ex) == 1 && strcmp(buf, "7") == 0 && ex);
  CK(s, cos_rat_decimal(R(22, 7), 6, buf, sizeof buf, &ex) == 8 && strcmp(buf, "3.142857") == 0 && !ex); /* cut off, not rounded: 3.1428571... */
  CK(s, cos_rat_decimal(R(2, 3), 3, buf, sizeof buf, &ex) == 5 && strcmp(buf, "0.666") == 0 && !ex);   /* not 0.667 */
  CK(s, cos_rat_decimal(R(1, 8), 10, buf, sizeof buf, &ex) == 5 && strcmp(buf, "0.125") == 0 && ex);
  CK(s, cos_rat_decimal(R(MAXV, 1), 3, buf, sizeof buf, &ex) == 19 && ex);
  CK(s, cos_rat_decimal(R(1, MAXV), 40, buf, sizeof buf, &ex) == 42 && !ex && strncmp(buf, "0.0000000000000000001084202172485504434", 39) == 0); /* 1/(2^63-1) */
  CK(s, cos_rat_decimal(R(1, 3), 41, buf, sizeof buf, &ex) == 0);                                  /* more than 40 places is refused */
  CK(s, cos_rat_decimal(R(1, 3), 5, buf, 7, &ex) == 0 && buf[0] == '\0');
  CK(s, cos_rat_decimal(R(1, 3), 5, buf, 8, &ex) == 7);
  CK(s, cos_rat_decimal(R(1, 3), 5, buf, 0, &ex) == 0);
  CK(s, cos_rat_decimal(R(1, 3), 5, buf, sizeof buf, NULL) == 7);
}

typedef struct Rng { uint64_t s; } Rng;
static uint64_t rnd(Rng *r) {
  r->s ^= r->s >> 12; r->s ^= r->s << 25; r->s ^= r->s >> 27;
  return r->s * 2685821657736338717ull;
}
static CosRat rnd_rat(Rng *r, unsigned bits) { /* a valid rational with numerator and denominator up to 2^bits */
  const uint64_t m = bits >= 63u ? (uint64_t)MAXV : ((uint64_t)1 << bits) - 1u;
  int64_t n = (int64_t)(rnd(r) % (m + 1u)), d = (int64_t)(rnd(r) % m) + 1;
  if ((rnd(r) & 1u) != 0u) n = -n;
  return R(n, d);
}

static void test_random(CosSuite *s) {
  Rng r = {0x2545F4914F6CDD1Dull};
  long bad = 0;
  /* operands up to 2^20: every result must exist and be valid; the three ways to get back to the start agree */
  for (int i = 0; i < 100000; ++i) {
    const CosRat a = rnd_rat(&r, 20), b = rnd_rat(&r, 20);
    CosRat t, u, v;
    if (cos_rat_add(a, b, &t) != COS_OK || !cos_rat_valid(t)) bad++;
    if (cos_rat_sub(t, b, &u) != COS_OK || cos_rat_cmp(u, a) != 0 || !eq(u, a.num, a.den)) bad++;
    if (cos_rat_mul(a, b, &t) != COS_OK || !cos_rat_valid(t)) bad++;
    if (b.num != 0) {
      if (cos_rat_div(t, b, &u) != COS_OK || !eq(u, a.num, a.den)) bad++;
      if (cos_rat_div(a, b, &t) != COS_OK || cos_rat_mul(t, b, &v) != COS_OK || !eq(v, a.num, a.den)) bad++;
    }
    /* distributive law, exact */
    const CosRat c = rnd_rat(&r, 10);
    CosRat l, rr, ab;
    if (cos_rat_add(a, b, &ab) != COS_OK || cos_rat_mul(c, ab, &l) != COS_OK) bad++;
    else {
      CosRat ca, cb;
      if (cos_rat_mul(c, a, &ca) != COS_OK || cos_rat_mul(c, b, &cb) != COS_OK || cos_rat_add(ca, cb, &rr) != COS_OK || !eq(l, rr.num, rr.den)) bad++;
    }
    /* text round trip */
    char buf[64];
    if (cos_rat_format(a, buf, sizeof buf) == 0) bad++;
  }
  CK(s, bad == 0);
  /* full range: whatever the operation says is either an exact valid answer or RANGE, never garbage; a refusal never alters the output */
  long refused = 0, answered = 0;
  bad = 0;
  for (int i = 0; i < 200000; ++i) {
    const CosRat a = rnd_rat(&r, 1u + (unsigned)(rnd(&r) % 63u)), b = rnd_rat(&r, 1u + (unsigned)(rnd(&r) % 63u));
    for (int op = 0; op < 4; ++op) {
      CosRat t = SENTINEL;
      CosStatus st = COS_OK;
      switch (op) {
        case 0: st = cos_rat_add(a, b, &t); break;
        case 1: st = cos_rat_sub(a, b, &t); break;
        case 2: st = cos_rat_mul(a, b, &t); break;
        default: st = cos_rat_div(a, b, &t); break;
      }
      if (st == COS_OK) { answered++; if (!cos_rat_valid(t)) bad++; }
      else if (st == COS_E_RANGE || (op == 3 && st == COS_E_INVALID && b.num == 0)) { refused++; if (!eq(t, 123456789, 987654321)) bad++; }
      else bad++;
    }
    /* cmp never fails and is consistent with sign of the difference when the difference exists */
    CosRat d;
    if (cos_rat_sub(a, b, &d) == COS_OK && cos_rat_cmp(a, b) != cos_rat_sign(d)) bad++;
    if (cos_rat_cmp(a, b) != -cos_rat_cmp(b, a)) bad++;
  }
  CK(s, bad == 0 && answered > 100000 && refused > 10000);
  printf("  random: %ld exact answers and %ld refusals on full-range operands, %ld violations\n", answered, refused, bad);
}

int main(void) {
  CosSuite s = {"c17_rat", 0, 0};
  test_make(&s);
  test_arithmetic(&s);
  test_pow(&s);
  test_cmp(&s);
  test_parse(&s);
  test_text(&s);
  test_random(&s);
  return suite_done(&s);
}
