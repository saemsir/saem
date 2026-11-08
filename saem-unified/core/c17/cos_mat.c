#include "cos_mat.h"

#include <string.h>

#include "cos_limits.h"

COS_STATIC_ASSERT(COS_WORKING_LIMIT == 32u, "matrix bounds and WTE contract must agree");

static bool overlaps(const void *a, size_t a_bytes, const void *b, size_t b_bytes) {
  const uintptr_t ap = (uintptr_t)a, bp = (uintptr_t)b;
  if (a_bytes == 0u || b_bytes == 0u) return false;
  if (ap > UINTPTR_MAX - a_bytes || bp > UINTPTR_MAX - b_bytes) return true;
  return ap < bp + b_bytes && bp < ap + a_bytes;
}

static bool zero(CosRat value) { return value.num == 0; }

static void swap_rows(CosRat *matrix, uint32_t cols, uint32_t a, uint32_t b) {
  if (a == b) return;
  for (uint32_t col = 0; col < cols; ++col) {
    const size_t ai = (size_t)a * cols + col;
    const size_t bi = (size_t)b * cols + col;
    const CosRat t = matrix[ai];
    matrix[ai] = matrix[bi];
    matrix[bi] = t;
  }
}

CosStatus cos_mat_rref(CosRat *matrix, uint32_t rows, uint32_t cols,
                       CosRat *scratch, size_t scratch_capacity,
                       uint32_t *pivot_columns, size_t pivot_capacity,
                       uint32_t *rank) {
  if (matrix == NULL || scratch == NULL || pivot_columns == NULL || rank == NULL ||
      rows == 0u || cols == 0u) return COS_E_INVALID;
  if (rows > COS_WORKING_LIMIT || cols > COS_WORKING_LIMIT) return COS_E_RANGE;

  const size_t count = (size_t)rows * cols;
  const size_t matrix_bytes = count * sizeof(*matrix);
  const size_t pivot_count = rows < cols ? rows : cols;
  const size_t pivot_bytes = pivot_count * sizeof(*pivot_columns);
  if (scratch_capacity < count || pivot_capacity < pivot_count) return COS_E_TOO_BIG;
  if (overlaps(matrix, matrix_bytes, scratch, matrix_bytes) ||
      overlaps(matrix, matrix_bytes, pivot_columns, pivot_bytes) ||
      overlaps(scratch, matrix_bytes, pivot_columns, pivot_bytes) ||
      overlaps(matrix, matrix_bytes, rank, sizeof(*rank)) ||
      overlaps(scratch, matrix_bytes, rank, sizeof(*rank)) ||
      overlaps(pivot_columns, pivot_bytes, rank, sizeof(*rank))) return COS_E_INVALID;

  for (size_t i = 0; i < count; ++i) {
    if (!cos_rat_valid(matrix[i])) return COS_E_INVALID;
  }
  memcpy(scratch, matrix, matrix_bytes);

  uint32_t pivots[COS_WORKING_LIMIT];
  uint32_t pivot_row = 0u;
  /* Gauss-Jordan row operations preserve Ax=b; pivot columns expose rank and row-space structure (Strang, CSE, p. 36). */
  for (uint32_t col = 0u; col < cols && pivot_row < rows; ++col) {
    uint32_t found = pivot_row;
    while (found < rows && zero(scratch[(size_t)found * cols + col])) ++found;
    if (found == rows) continue;

    swap_rows(scratch, cols, pivot_row, found);
    const size_t pivot_index = (size_t)pivot_row * cols + col;
    const CosRat pivot = scratch[pivot_index];
    for (uint32_t j = 0u; j < cols; ++j) {
      CosRat value;
      const CosStatus st = cos_rat_div(scratch[(size_t)pivot_row * cols + j], pivot, &value);
      if (st != COS_OK) return st;
      scratch[(size_t)pivot_row * cols + j] = value;
    }

    for (uint32_t row = 0u; row < rows; ++row) {
      if (row == pivot_row) continue;
      const size_t row_start = (size_t)row * cols;
      const CosRat factor = scratch[row_start + col];
      if (zero(factor)) continue;
      for (uint32_t j = 0u; j < cols; ++j) {
        CosRat product, value;
        CosStatus st = cos_rat_mul(factor, scratch[(size_t)pivot_row * cols + j], &product);
        if (st != COS_OK) return st;
        st = cos_rat_sub(scratch[row_start + j], product, &value);
        if (st != COS_OK) return st;
        scratch[row_start + j] = value;
      }
    }
    pivots[pivot_row] = col;
    ++pivot_row;
  }

  memcpy(matrix, scratch, matrix_bytes);
  memcpy(pivot_columns, pivots, (size_t)pivot_row * sizeof(*pivot_columns));
  *rank = pivot_row;
  return COS_OK;
}
