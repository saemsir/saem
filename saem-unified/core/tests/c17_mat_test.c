#include <stdint.h>
#include <string.h>

#include "c17_check.h"
#include "cos_mat.h"

static int eq(CosRat a, int64_t n, int64_t d) { return a.num == n && a.den == d; }

static void test_rref(CosSuite *s) {
  CosRat a[9] = { {1,1}, {2,1}, {1,1}, {2,1}, {4,1}, {0,1}, {0,1}, {0,1}, {1,1} };
  CosRat original[9];
  CosRat work[9];
  uint32_t pivots[3] = {99u, 99u, 99u}, rank = 99u;
  memcpy(original, a, sizeof(a));
  CK(s, cos_mat_rref(a, 3u, 3u, work, 9u, pivots, 3u, &rank) == COS_OK);
  CK(s, rank == 2u && pivots[0] == 0u && pivots[1] == 2u);
  CK(s, eq(a[0], 1, 1) && eq(a[1], 2, 1) && eq(a[2], 0, 1));
  CK(s, eq(a[3], 0, 1) && eq(a[4], 0, 1) && eq(a[5], 1, 1));
  CK(s, eq(a[6], 0, 1) && eq(a[7], 0, 1) && eq(a[8], 0, 1));
  CK(s, memcmp(original, a, sizeof(a)) != 0);

  CosRat rectangular[6] = { {0,1}, {0,1}, {1,1}, {0,1}, {0,1}, {0,1} };
  uint32_t wide_pivots[2] = {99u, 99u};
  CK(s, cos_mat_rref(rectangular, 2u, 3u, work, 6u, wide_pivots, 2u, &rank) == COS_OK);
  CK(s, rank == 1u && wide_pivots[0] == 2u);
  CK(s, eq(rectangular[2], 1, 1) && eq(rectangular[3], 0, 1));

  CosRat zero_matrix[4] = { {0,1}, {0,1}, {0,1}, {0,1} };
  uint32_t zero_pivots[2] = {99u, 99u};
  CK(s, cos_mat_rref(zero_matrix, 2u, 2u, work, 4u, zero_pivots, 2u, &rank) == COS_OK);
  CK(s, rank == 0u && zero_pivots[0] == 99u && zero_pivots[1] == 99u);
}

static void test_refusals(CosSuite *s) {
  CosRat a[4] = { {1,1}, {INT64_MAX,1}, {1,1}, {-INT64_MAX,1} };
  CosRat before[4], work[4];
  uint32_t pivots[2] = {17u, 18u}, rank = 19u;
  memcpy(before, a, sizeof(a));
  CK(s, cos_mat_rref(a, 2u, 2u, work, 4u, pivots, 2u, &rank) == COS_E_RANGE);
  CK(s, memcmp(a, before, sizeof(a)) == 0);
  CK(s, pivots[0] == 17u && pivots[1] == 18u && rank == 19u);

  CosRat bad[1] = { {0,2} }, bad_work[1] = { {0,1} };
  uint32_t one_pivot = 23u, one_rank = 24u;
  CK(s, cos_mat_rref(bad, 1u, 1u, bad_work, 1u, &one_pivot, 1u, &one_rank) == COS_E_INVALID);
  CK(s, eq(bad[0], 0, 2) && eq(bad_work[0], 0, 1) && one_pivot == 23u && one_rank == 24u);
  CK(s, cos_mat_rref(a, 2u, 2u, work, 3u, pivots, 2u, &rank) == COS_E_TOO_BIG);
  CK(s, cos_mat_rref(a, 2u, 2u, work, 4u, pivots, 1u, &rank) == COS_E_TOO_BIG);
  CK(s, cos_mat_rref(a, 33u, 1u, work, 4u, pivots, 2u, &rank) == COS_E_RANGE);
  CK(s, cos_mat_rref(a, 2u, 2u, a, 4u, pivots, 2u, &rank) == COS_E_INVALID);
  CK(s, cos_mat_rref(a, 2u, 2u, work, 4u, pivots, 2u, pivots) == COS_E_INVALID);
  CK(s, cos_mat_rref(NULL, 2u, 2u, work, 4u, pivots, 2u, &rank) == COS_E_INVALID);
}

int main(void) {
  CosSuite suite = {"exact matrices", 0, 0};
  test_rref(&suite);
  test_refusals(&suite);
  return suite_done(&suite);
}
