#include <stdint.h>
#include <stdio.h>

#include "cos_limits.h"
#include "cos_mat.h"

int main(void) {
  char mode;
  while (scanf(" %c", &mode) == 1) {
    uint32_t rows, cols;
    if (mode == 'i') {
      if (scanf("%u", &rows) != 1) return 2;
      cols = rows;
    } else if (mode == 's') {
      if (scanf("%u %u", &rows, &cols) != 2) return 2;
    } else {
      return 2;
    }
    if (rows == 0u || cols == 0u || rows > COS_WORKING_LIMIT ||
        cols > COS_WORKING_LIMIT) return 2;
    CosRat matrix[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat rhs[COS_WORKING_LIMIT], solution[COS_WORKING_LIMIT];
    CosRat inverse[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat reduced[COS_WORKING_LIMIT * (COS_WORKING_LIMIT + 1u)];
    CosRat scratch[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    uint32_t pivots[COS_WORKING_LIMIT], rank = UINT32_MAX;
    CosMatWorkspace workspace = {
      reduced, COS_WORKING_LIMIT * (COS_WORKING_LIMIT + 1u),
      scratch, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
      pivots, COS_WORKING_LIMIT
    };
    for (size_t i = 0u; i < (size_t)rows * cols; ++i) {
      long long n, d;
      if (scanf("%lld %lld", &n, &d) != 2 ||
          cos_rat_make((int64_t)n, (int64_t)d, &matrix[i]) != COS_OK) return 2;
    }
    CosStatus st;
    if (mode == 's') {
      for (uint32_t row = 0u; row < rows; ++row) {
        long long n, d;
        if (scanf("%lld %lld", &n, &d) != 2 ||
            cos_rat_make((int64_t)n, (int64_t)d, &rhs[row]) != COS_OK) return 2;
      }
      st = cos_mat_solve(matrix, rows, cols, rhs, solution, COS_WORKING_LIMIT,
                         &rank, &workspace);
      printf("%d", (int)st);
      if (st == COS_OK) {
        printf(" %u", rank);
        for (uint32_t i = 0u; i < cols; ++i) printf(" %lld/%lld", (long long)solution[i].num, (long long)solution[i].den);
      }
    } else {
      st = cos_mat_inverse(matrix, rows, inverse, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                           &workspace);
      printf("%d", (int)st);
      if (st == COS_OK) {
        for (size_t i = 0u; i < (size_t)rows * rows; ++i) {
          printf(" %lld/%lld", (long long)inverse[i].num, (long long)inverse[i].den);
        }
      }
    }
    putchar('\n');
  }
  return 0;
}
