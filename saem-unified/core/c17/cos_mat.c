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

static CosStatus validate_workspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                                    CosRat *basis, size_t basis_capacity,
                                    uint32_t *basis_dimension, CosMatWorkspace *workspace,
                                    size_t basis_required) {
  if (matrix == NULL || basis == NULL || basis_dimension == NULL || workspace == NULL ||
      workspace->reduced == NULL || workspace->scratch == NULL || workspace->pivot_columns == NULL ||
      rows == 0u || cols == 0u) return COS_E_INVALID;
  if (rows > COS_WORKING_LIMIT || cols > COS_WORKING_LIMIT) return COS_E_RANGE;
  if (basis_capacity < basis_required) return COS_E_TOO_BIG;

  const size_t count = (size_t)rows * cols;
  const size_t matrix_bytes = count * sizeof(*matrix);
  const size_t reduced_bytes = count * sizeof(*workspace->reduced);
  const size_t pivot_count = rows < cols ? rows : cols;
  const size_t pivot_bytes = pivot_count * sizeof(*workspace->pivot_columns);
  const size_t basis_bytes = basis_required * sizeof(*basis);
  const size_t dimension_bytes = sizeof(*basis_dimension);
  if (workspace->reduced_capacity < count || workspace->scratch_capacity < count ||
      workspace->pivot_capacity < pivot_count) return COS_E_TOO_BIG;
  if (overlaps(matrix, matrix_bytes, basis, basis_bytes) ||
      overlaps(matrix, matrix_bytes, workspace->reduced, reduced_bytes) ||
      overlaps(matrix, matrix_bytes, workspace->scratch, reduced_bytes) ||
      overlaps(matrix, matrix_bytes, workspace->pivot_columns, pivot_bytes) ||
      overlaps(basis, basis_bytes, workspace->reduced, reduced_bytes) ||
      overlaps(basis, basis_bytes, workspace->scratch, reduced_bytes) ||
      overlaps(basis, basis_bytes, workspace->pivot_columns, pivot_bytes) ||
      overlaps(workspace->reduced, reduced_bytes, workspace->scratch, reduced_bytes) ||
      overlaps(workspace->reduced, reduced_bytes, workspace->pivot_columns, pivot_bytes) ||
      overlaps(workspace->scratch, reduced_bytes, workspace->pivot_columns, pivot_bytes) ||
      overlaps(workspace, sizeof(*workspace), matrix, matrix_bytes) ||
      overlaps(workspace, sizeof(*workspace), basis, basis_bytes) ||
      overlaps(workspace, sizeof(*workspace), workspace->reduced, reduced_bytes) ||
      overlaps(workspace, sizeof(*workspace), workspace->scratch, reduced_bytes) ||
      overlaps(workspace, sizeof(*workspace), workspace->pivot_columns, pivot_bytes) ||
      overlaps(workspace, sizeof(*workspace), basis_dimension, dimension_bytes) ||
      overlaps(basis_dimension, dimension_bytes, matrix, matrix_bytes) ||
      overlaps(basis_dimension, dimension_bytes, basis, basis_bytes) ||
      overlaps(basis_dimension, dimension_bytes, workspace->reduced, reduced_bytes) ||
      overlaps(basis_dimension, dimension_bytes, workspace->scratch, reduced_bytes) ||
      overlaps(basis_dimension, dimension_bytes, workspace->pivot_columns, pivot_bytes)) return COS_E_INVALID;
  return COS_OK;
}

static CosStatus basis_rref(const CosRat *matrix, uint32_t rows, uint32_t cols,
                            CosMatWorkspace *workspace, bool transpose, uint32_t *rank) {
  const uint32_t rref_rows = transpose ? cols : rows;
  const uint32_t rref_cols = transpose ? rows : cols;
  if (transpose) {
    for (uint32_t row = 0u; row < cols; ++row) {
      for (uint32_t col = 0u; col < rows; ++col) {
        workspace->reduced[(size_t)row * rows + col] = matrix[(size_t)col * cols + row];
      }
    }
  } else {
    memcpy(workspace->reduced, matrix, (size_t)rows * cols * sizeof(*matrix));
  }
  return cos_mat_rref(workspace->reduced, rref_rows, rref_cols,
                      workspace->scratch, workspace->scratch_capacity,
                      workspace->pivot_columns, workspace->pivot_capacity, rank);
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

static CosStatus nullspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                           CosRat *basis, size_t basis_capacity,
                           uint32_t *basis_dimension, CosMatWorkspace *workspace,
                           bool transpose) {
  const uint32_t ambient = transpose ? rows : cols;
  const size_t basis_required = (size_t)ambient * ambient;
  CosStatus st = validate_workspace(matrix, rows, cols, basis, basis_capacity,
                                    basis_dimension, workspace, basis_required);
  if (st != COS_OK) return st;

  uint32_t rank;
  st = basis_rref(matrix, rows, cols, workspace, transpose, &rank);
  if (st != COS_OK) return st;

  const uint32_t dim = ambient - rank;
  const uint32_t rref_cols = ambient;
  size_t out = 0u;
  /* The free variables give dim N(A)=n-r, and pivot variables are solved from RREF (Strang, CSE, pp. 37-38). */
  for (uint32_t free_col = 0u; free_col < ambient; ++free_col) {
    bool is_pivot = false;
    for (uint32_t row = 0u; row < rank; ++row) {
      if (workspace->pivot_columns[row] == free_col) is_pivot = true;
    }
    if (is_pivot) continue;

    for (uint32_t col = 0u; col < ambient; ++col) basis[out + col] = (CosRat){0, 1};
    basis[out + free_col] = (CosRat){1, 1};
    for (uint32_t row = 0u; row < rank; ++row) {
      const uint32_t pivot_col = workspace->pivot_columns[row];
      const CosRat value = workspace->reduced[(size_t)row * rref_cols + free_col];
      basis[out + pivot_col].num = -value.num;
      basis[out + pivot_col].den = value.den;
    }
    out += ambient;
  }
  *basis_dimension = dim;
  return COS_OK;
}

CosStatus cos_mat_nullspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                            CosRat *basis, size_t basis_capacity,
                            uint32_t *basis_dimension, CosMatWorkspace *workspace) {
  return nullspace(matrix, rows, cols, basis, basis_capacity, basis_dimension, workspace, false);
}

CosStatus cos_mat_left_nullspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                                 CosRat *basis, size_t basis_capacity,
                                 uint32_t *basis_dimension, CosMatWorkspace *workspace) {
  return nullspace(matrix, rows, cols, basis, basis_capacity, basis_dimension, workspace, true);
}

CosStatus cos_mat_rowspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                           CosRat *basis, size_t basis_capacity,
                           uint32_t *basis_dimension, CosMatWorkspace *workspace) {
  const size_t basis_required = (size_t)cols * cols;
  CosStatus st = validate_workspace(matrix, rows, cols, basis, basis_capacity,
                                    basis_dimension, workspace, basis_required);
  if (st != COS_OK) return st;
  uint32_t rank;
  st = basis_rref(matrix, rows, cols, workspace, false, &rank);
  if (st != COS_OK) return st;
  memcpy(basis, workspace->reduced, (size_t)rank * cols * sizeof(*basis));
  *basis_dimension = rank;
  return COS_OK;
}

CosStatus cos_mat_columnspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                              CosRat *basis, size_t basis_capacity,
                              uint32_t *basis_dimension, CosMatWorkspace *workspace) {
  const size_t basis_required = (size_t)rows * rows;
  CosStatus st = validate_workspace(matrix, rows, cols, basis, basis_capacity,
                                    basis_dimension, workspace, basis_required);
  if (st != COS_OK) return st;
  uint32_t rank;
  st = basis_rref(matrix, rows, cols, workspace, false, &rank);
  if (st != COS_OK) return st;
  for (uint32_t vector = 0u; vector < rank; ++vector) {
    const uint32_t source_col = workspace->pivot_columns[vector];
    for (uint32_t row = 0u; row < rows; ++row) {
      basis[(size_t)vector * rows + row] = matrix[(size_t)row * cols + source_col];
    }
  }
  *basis_dimension = rank;
  return COS_OK;
}
