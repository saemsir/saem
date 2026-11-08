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

#ifdef __cplusplus
}
#endif
#endif
