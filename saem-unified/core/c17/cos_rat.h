/* Exact rational numbers: the only numbers the Semantic IR knows. Zero numerical approximation: a value is held exactly or it is refused.
 *
 *   invariant of every valid CosRat: den > 0, gcd(|num|, den) = 1 (so zero is 0/1), and num, den are in [-INT64_MAX, INT64_MAX] (INT64_MIN is not
 *   a value: it keeps negation total).
 *
 * Every operation either returns the exact answer or a status, never a rounded one:
 *   COS_E_RANGE    the exact result does not fit in 64 bits. mul and div decide this exactly (an answer that fits is always returned: both
 *                  factors are cross-reduced before multiplying). add and sub also refuse when the numerator before the last cancellation does
 *                  not fit in 63 bits, so in contrived cases an exactly representable sum is refused, e.g. (2^63-1)/2^62 + 3/2^62 (never a
 *                  wrong answer, only a refusal: the Forbidden Zone is fail-closed). pow refuses when a square on the way does not fit.
 *   COS_E_INVALID  division by zero, 0^0, 0 to a negative power, a malformed number, or an argument that is not a valid CosRat
 * On error *out is left unchanged. No heap, no floating point, no global state; pure functions of their arguments.
 * Adapted from Knuth, TAOCP vol. 2, 4.5.1 (greatest common divisors before products). cmp is exact for every valid pair (128-bit products).
 * C17, written for the Saem sovereign kernel. */
#ifndef COS_RAT_H
#define COS_RAT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cos_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CosRat {
  int64_t num;
  int64_t den;
} CosRat;

bool cos_rat_valid(CosRat a);                               /* the invariant above */
CosStatus cos_rat_make(int64_t num, int64_t den, CosRat *out); /* reduces; den 0 is COS_E_INVALID; INT64_MIN anywhere is COS_E_RANGE */
CosStatus cos_rat_int(int64_t v, CosRat *out);

CosStatus cos_rat_add(CosRat a, CosRat b, CosRat *out);
CosStatus cos_rat_sub(CosRat a, CosRat b, CosRat *out);
CosStatus cos_rat_mul(CosRat a, CosRat b, CosRat *out);
CosStatus cos_rat_div(CosRat a, CosRat b, CosRat *out);
CosStatus cos_rat_neg(CosRat a, CosRat *out);
CosStatus cos_rat_abs(CosRat a, CosRat *out);
CosStatus cos_rat_pow(CosRat a, int32_t e, CosRat *out);    /* integer power; negative e inverts */

int cos_rat_cmp(CosRat a, CosRat b);                        /* -1, 0, 1: exact for every valid pair, no overflow possible */
int cos_rat_sign(CosRat a);
bool cos_rat_is_int(CosRat a);

/* The JSON number grammar, exactly: -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?  and nothing else (no leading plus, no leading zeros, no
 * ".5", no "NaN", no spaces). The value is read as a decimal fraction, so 0.1 is exactly 1/10. Refused as COS_E_RANGE: a mantissa (the digits with leading
 * and trailing zeros dropped) that needs more than 64 bits (20 digits or more, depending on their value), or a reduced value that does not fit
 * in the range above. Any exponent is accepted for a zero mantissa. Anything malformed is COS_E_INVALID. */
CosStatus cos_rat_parse(const uint8_t *s, size_t n, CosRat *out);

/* Canonical exact text: "-12" for an integer, "-3/4" otherwise. Returns the length written (NUL-terminated), or 0 if cap is too small. */
size_t cos_rat_format(CosRat a, char *out, size_t cap);
/* DISPLAY ONLY: the decimal expansion cut off (toward zero, not rounded) after `places` digits (at most 40). An expansion that ends by itself is
 * printed without padding zeros; "-" is printed only if a nonzero digit is. *exact is true when the expansion ended by itself, i.e. what is
 * printed IS the value. Returns the length, or 0 if cap is too small or places > 40. */
size_t cos_rat_decimal(CosRat a, uint32_t places, char *out, size_t cap, bool *exact);

#ifdef __cplusplus
}
#endif
#endif
