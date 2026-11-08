#include <stdint.h>
#include <stdio.h>

#include "cos_limits.h"
#include "cos_mat.h"

int main(void) {
  uint32_t rows, cols;
  while (scanf("%u %u", &rows, &cols) == 2) {
    if (rows == 0u || cols == 0u || rows > COS_WORKING_LIMIT || cols > COS_WORKING_LIMIT) return 2;
    CosRat matrix[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat reduced[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat scratch[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat null_basis[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat row_basis[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat column_basis[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat left_basis[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    uint32_t pivots[COS_WORKING_LIMIT];
    uint32_t null_dim = UINT32_MAX, row_dim = UINT32_MAX;
    uint32_t column_dim = UINT32_MAX, left_dim = UINT32_MAX;
    for (size_t i = 0u; i < (size_t)rows * cols; ++i) {
      long long n, d;
      if (scanf("%lld %lld", &n, &d) != 2 ||
          cos_rat_make((int64_t)n, (int64_t)d, &matrix[i]) != COS_OK) return 2;
    }
    CosMatWorkspace workspace = {
      reduced, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
      scratch, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
      pivots, COS_WORKING_LIMIT
    };
    CosStatus st = cos_mat_nullspace(matrix, rows, cols, null_basis, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                                    &null_dim, &workspace);
    if (st == COS_OK) st = cos_mat_rowspace(matrix, rows, cols, row_basis, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                                             &row_dim, &workspace);
    if (st == COS_OK) st = cos_mat_columnspace(matrix, rows, cols, column_basis, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                                                &column_dim, &workspace);
    if (st == COS_OK) st = cos_mat_left_nullspace(matrix, rows, cols, left_basis, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                                                   &left_dim, &workspace);
    printf("%d", (int)st);
    if (st == COS_OK) {
      printf(" %u %u %u %u", null_dim, row_dim, column_dim, left_dim);
      for (size_t i = 0u; i < (size_t)null_dim * cols; ++i) printf(" %lld/%lld", (long long)null_basis[i].num, (long long)null_basis[i].den);
      for (size_t i = 0u; i < (size_t)row_dim * cols; ++i) printf(" %lld/%lld", (long long)row_basis[i].num, (long long)row_basis[i].den);
      for (size_t i = 0u; i < (size_t)column_dim * rows; ++i) printf(" %lld/%lld", (long long)column_basis[i].num, (long long)column_basis[i].den);
      for (size_t i = 0u; i < (size_t)left_dim * rows; ++i) printf(" %lld/%lld", (long long)left_basis[i].num, (long long)left_basis[i].den);
    }
    putchar('\n');
  }
  return 0;
}
