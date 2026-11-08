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
 * length. Provide cols*cols entries for nullspace/rowspace, rows*min(rows,cols)
 * for columnspace, and rows*rows for left_nullspace. Workspace buffers must be
 * distinct from each other, the input, and basis. */
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

/* Solve A*x=b exactly. For consistent underdetermined systems, returns the
 * particular solution with free variables set to zero; rank describes the
 * system and nullspace APIs describe its homogeneous degrees of freedom.
 * reduced_capacity must be at least rows*(cols+1); scratch_capacity at least
 * rows*cols. Inputs, outputs, and workspace buffers must not overlap. */
CosStatus cos_mat_solve(const CosRat *matrix, uint32_t rows, uint32_t cols,
                        const CosRat *rhs, CosRat *solution, size_t solution_capacity,
                        uint32_t *rank, CosMatWorkspace *workspace);

/* Exact inverse of a square matrix. reduced_capacity must be at least
 * n*(n+1), scratch_capacity at least n*n. Output is untouched unless invertible. */
CosStatus cos_mat_inverse(const CosRat *matrix, uint32_t n,
                          CosRat *inverse, size_t inverse_capacity,
                          CosMatWorkspace *workspace);

/* Orthogonal projector onto C(matrix). Uses an independent pivot-column basis,
 * so rank-deficient matrices are supported. Output is row-major rows*rows.
 * staging_capacity >= rows*rows, basis_capacity >= rows*min(rows,cols), and
 * gram_capacity >= min(rows,cols)^2. Workspace reduced storage must fit both
 * rows*cols and r*(r+1), scratch rows*cols, and pivots min(rows,cols). All
 * supplied buffers, inputs, outputs, and the workspace descriptor are disjoint. */
CosStatus cos_mat_projection(const CosRat *matrix, uint32_t rows, uint32_t cols,
                             CosRat *projection, size_t projection_capacity,
                             CosRat *staging, size_t staging_capacity,
                             CosRat *basis, size_t basis_capacity,
                             CosRat *gram, size_t gram_capacity,
                             CosMatWorkspace *workspace);

#ifdef __cplusplus
}
#endif
#endif
