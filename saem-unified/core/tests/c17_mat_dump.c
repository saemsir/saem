#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cos_limits.h"
#include "cos_mat.h"

int main(void) {
  uint32_t rows, cols;
  while (scanf("%u %u", &rows, &cols) == 2) {
    if (rows == 0u || cols == 0u || rows > COS_WORKING_LIMIT || cols > COS_WORKING_LIMIT) return 2;
    const size_t count = (size_t)rows * cols;
    CosRat matrix[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    CosRat scratch[COS_WORKING_LIMIT * COS_WORKING_LIMIT];
    uint32_t pivots[COS_WORKING_LIMIT], rank = UINT32_MAX;
    for (size_t i = 0; i < count; ++i) {
      long long n, d;
      if (scanf("%lld %lld", &n, &d) != 2) return 2;
      if (cos_rat_make((int64_t)n, (int64_t)d, &matrix[i]) != COS_OK) return 2;
    }
    const CosStatus st = cos_mat_rref(matrix, rows, cols, scratch, COS_WORKING_LIMIT * COS_WORKING_LIMIT,
                                      pivots, COS_WORKING_LIMIT, &rank);
    if (st != COS_OK) {
      printf("%d\n", (int)st);
      continue;
    }
    printf("%d %u", (int)st, rank);
    for (uint32_t i = 0; i < rank; ++i) printf(" %u", pivots[i]);
    for (size_t i = 0; i < count; ++i) printf(" %lld/%lld", (long long)matrix[i].num, (long long)matrix[i].den);
    putchar('\n');
  }
  return 0;
}
