#include <stdint.h>
#include <stdio.h>

#include "cos_limits.h"
#include "cos_mat.h"

int main(void) {
  uint32_t rows, cols;
  while (scanf("%u %u", &rows, &cols) == 2) {
    if (rows == 0u || cols == 0u || rows > COS_WORKING_LIMIT || cols > COS_WORKING_LIMIT) return 2;
    CosRat matrix[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat projection[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat staging[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat basis[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat gram[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat reduced[COS_WORKING_LIMIT * (COS_WORKING_LIMIT + 1u)];
    CosRat scratch[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    uint32_t pivots[COS_WORKING_LIMIT];
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
    const CosStatus st = cos_mat_projection(
        matrix, rows, cols, projection, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
        staging, COS_WORKING_LIMIT * COS_WORKING_LIMIT, basis,
        COS_WORKING_LIMIT * COS_WORKING_LIMIT, gram,
        COS_WORKING_LIMIT * COS_WORKING_LIMIT, &workspace);
    printf("%d", (int)st);
    if (st == COS_OK) {
      for (size_t i = 0u; i < (size_t)rows * rows; ++i) {
        printf(" %lld/%lld", (long long)projection[i].num, (long long)projection[i].den);
      }
    }
    putchar('\n');
  }
  return 0;
}
