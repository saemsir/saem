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

static int dot_is_zero(const CosRat *a, const CosRat *b, uint32_t n) {
  CosRat sum = {0, 1};
  for (uint32_t i = 0u; i < n; ++i) {
    CosRat product, next;
    if (cos_rat_mul(a[i], b[i], &product) != COS_OK ||
        cos_rat_add(sum, product, &next) != COS_OK) return 0;
    sum = next;
  }
  return sum.num == 0;
}

static void test_four_spaces(CosSuite *s) {
  const CosRat a[6] = { {1,1}, {2,1}, {1,1}, {2,1}, {4,1}, {2,1} };
  CosRat reduced[6], scratch[6], basis[9];
  uint32_t pivots[3], dim = 99u;
  CosMatWorkspace workspace = {reduced, 6u, scratch, 6u, pivots, 3u};

  CK(s, cos_mat_nullspace(a, 2u, 3u, basis, 9u, &dim, &workspace) == COS_OK);
  CK(s, dim == 2u);
  CK(s, eq(basis[0], -2, 1) && eq(basis[1], 1, 1) && eq(basis[2], 0, 1));
  CK(s, eq(basis[3], -1, 1) && eq(basis[4], 0, 1) && eq(basis[5], 1, 1));

  CK(s, cos_mat_rowspace(a, 2u, 3u, basis, 9u, &dim, &workspace) == COS_OK);
  CK(s, dim == 1u && eq(basis[0], 1, 1) && eq(basis[1], 2, 1) && eq(basis[2], 1, 1));
  CK(s, cos_mat_columnspace(a, 2u, 3u, basis, 4u, &dim, &workspace) == COS_OK);
  CK(s, dim == 1u && eq(basis[0], 1, 1) && eq(basis[1], 2, 1));

  CK(s, cos_mat_left_nullspace(a, 2u, 3u, basis, 4u, &dim, &workspace) == COS_OK);
  CK(s, dim == 1u && eq(basis[0], -2, 1) && eq(basis[1], 1, 1));

  CosRat row_basis[9], null_basis[9], col_basis[4], left_basis[4];
  uint32_t row_dim = 0u, null_dim = 0u, col_dim = 0u, left_dim = 0u;
  CK(s, cos_mat_rowspace(a, 2u, 3u, row_basis, 9u, &row_dim, &workspace) == COS_OK);
  CK(s, cos_mat_nullspace(a, 2u, 3u, null_basis, 9u, &null_dim, &workspace) == COS_OK);
  CK(s, cos_mat_columnspace(a, 2u, 3u, col_basis, 4u, &col_dim, &workspace) == COS_OK);
  CK(s, cos_mat_left_nullspace(a, 2u, 3u, left_basis, 4u, &left_dim, &workspace) == COS_OK);
  CK(s, row_dim == col_dim && row_dim + null_dim == 3u && col_dim + left_dim == 2u);
  for (uint32_t i = 0u; i < row_dim; ++i)
    for (uint32_t j = 0u; j < null_dim; ++j)
      CK(s, dot_is_zero(&row_basis[(size_t)i * 3u], &null_basis[(size_t)j * 3u], 3u));
  for (uint32_t i = 0u; i < col_dim; ++i)
    for (uint32_t j = 0u; j < left_dim; ++j)
      CK(s, dot_is_zero(&col_basis[(size_t)i * 2u], &left_basis[(size_t)j * 2u], 2u));

  CosRat sentinel[9];
  for (size_t i = 0u; i < 9u; ++i) sentinel[i] = (CosRat){77, 1};
  dim = 88u;
  CK(s, cos_mat_nullspace(a, 2u, 3u, sentinel, 8u, &dim, &workspace) == COS_E_TOO_BIG);
  CK(s, dim == 88u && eq(sentinel[0], 77, 1));

  const CosRat growth[4] = { {1,1}, {INT64_MAX,1}, {1,1}, {-INT64_MAX,1} };
  CosRat growth_reduced[4], growth_scratch[4], growth_basis[4];
  uint32_t growth_pivots[2], growth_dim = 31u;
  for (size_t i = 0u; i < 4u; ++i) growth_basis[i] = (CosRat){77, 1};
  CosMatWorkspace growth_workspace = {growth_reduced, 4u, growth_scratch, 4u, growth_pivots, 2u};
  CK(s, cos_mat_nullspace(growth, 2u, 2u, growth_basis, 4u, &growth_dim, &growth_workspace) == COS_E_RANGE);
  CK(s, growth_dim == 31u && eq(growth_basis[0], 77, 1) && eq(growth_basis[3], 77, 1));

  CosMatWorkspace aliased_workspace = {growth_reduced, 4u, growth_reduced, 4u, growth_pivots, 2u};
  CK(s, cos_mat_nullspace(growth, 2u, 2u, growth_basis, 4u, &growth_dim, &aliased_workspace) == COS_E_INVALID);
}

int main(void) {
  CosSuite suite = {"exact matrices", 0, 0};
  test_rref(&suite);
  test_refusals(&suite);
  test_four_spaces(&suite);
  return suite_done(&suite);
}
