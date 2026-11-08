/* Exact rational matrix primitives for the Work-Time Engine.
 * Matrices are row-major, bounded by COS_WORKING_LIMIT, and stored by the caller.
 * No allocation or approximation; on failure the matrix and metadata are untouched. */
#ifndef COS_MAT_H
#define COS_MAT_H

#include <stddef.h>
#include <stdint.h>

#include "cos_limits.h"
#include "cos_rat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RREF in place. scratch holds rows*cols entries and must not overlap matrix.
 * pivot_columns holds min(rows, cols) entries; rank and pivot outputs are committed only on success.
 * Rows and columns must both be in [1, COS_WORKING_LIMIT]. */
CosStatus cos_mat_rref(CosRat *matrix, uint32_t rows, uint32_t cols,
                       CosRat *scratch, size_t scratch_capacity,
                       uint32_t *pivot_columns, size_t pivot_capacity,
                       uint32_t *rank);

typedef struct CosMatWorkspace {
  CosRat *reduced;
  size_t reduced_capacity;
  CosRat *scratch;
  size_t scratch_capacity;
  uint32_t *pivot_columns;
  size_t pivot_capacity;
} CosMatWorkspace;

/* Each basis is stored as consecutive row vectors in `basis`; basis_dimension
 * is the number of vectors, and every vector has the ambient dimension as its
 * length. Provide cols*cols entries for nullspace/rowspace and rows*rows for
 * columnspace/left_nullspace. Workspace buffers must be distinct from each
 * other, the input, and basis. */
CosStatus cos_mat_nullspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                            CosRat *basis, size_t basis_capacity,
                            uint32_t *basis_dimension, CosMatWorkspace *workspace);
CosStatus cos_mat_rowspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                           CosRat *basis, size_t basis_capacity,
                           uint32_t *basis_dimension, CosMatWorkspace *workspace);
CosStatus cos_mat_columnspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                              CosRat *basis, size_t basis_capacity,
                              uint32_t *basis_dimension, CosMatWorkspace *workspace);
CosStatus cos_mat_left_nullspace(const CosRat *matrix, uint32_t rows, uint32_t cols,
                                 CosRat *basis, size_t basis_capacity,
                                 uint32_t *basis_dimension, CosMatWorkspace *workspace);

#ifdef __cplusplus
}
#endif
#endif
