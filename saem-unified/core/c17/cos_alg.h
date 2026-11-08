/* The number tower of the Work-Time Engine (cos_alg): exact Cayley-Dickson algebras over the exact rationals of cos_rat.
 *
 *   level 0   Q          rationals               dim 1   commutative, associative       a field (Q is the fraction field of Z)
 *   level 1   Q(i)       Gaussian rationals      dim 2   commutative, associative       a field, Q[x]/(x^2+1): -1 is not a square in Q
 *   level 2   H over Q   Hamilton's quaternions  dim 4   NOT commutative                a division ring with centre Q (the order of work matters)
 *   level 3   O over Q   octonions               dim 8   NOT commutative, NOT associative, but alternative: a division algebra with centre Q
 *                                                        (the grouping of work matters)
 *   level 4+  REFUSED (COS_E_INVALID): the sedenions have zero divisors and the norm stops being multiplicative, so the tower ends here.
 *             tools/check_alg.py finds an explicit pair a b = 0 at level 4, and the 40-of-40 test of the norm rule failing there.
 *
 * An element is 8 components c[0..dim-1] in the basis e_0 = 1, e_1, ..., e_{dim-1}; the unused components are exactly 0 (canonical form).
 * Doubling rule (Cayley-Dickson): a level-L element is a pair (a, b) of level-(L-1) elements, with
 *     (a, b)(c, d) = (a c - conj(d) b,  d a + b conj(c)),     conj(a, b) = (conj(a), -b),
 * and the basis element e_i for i < 2^(L-1) is (e_i, 0), so each level sits inside the next by copying components (cos_alg_embed).
 * (Baez, section 2.2, writes the mirror convention (a,b)(c,d) = (ac - d conj(b), conj(a) d + c b); the map (a,b) -> (psi(a), conj(psi(b))) is an explicit
 * isomorphism between the two, checked by tools/check_alg.py.) Every basis product is a signed basis element, e_i e_j = sign(i, j) e_{i xor j};
 * the sign comes from the four-case rule in cos_alg.c, and the independent oracle (tools/alg_oracle.py) multiplies by the pair rule above instead.
 *
 * Integrity contract, each line tested in core/tests/c17_alg_test.c and again against the oracle on thousands of random operands:
 *   ring        + is an abelian group; the product is two-sided distributive over +, has the unit e_0, and commutes with rational scalars (Q-algebra).
 *               Associative at levels <= 2; at level 3 it fails (explicit witness) and what replaces it is ALTERNATIVITY: (aa)b = a(ab), (ba)a = b(aa),
 *               flexibility (ab)a = a(ba), the associator changes sign under every transposition, and the three Moufang identities hold. By Emil Artin's
 *               theorem (Baez, equation (1)) any two elements generate an associative subalgebra: (a, b, ab) and (a, ab, ba) associate.
 *   tower       cos_alg_embed is an injective unital homomorphism from level k into level k+1: it keeps +, *, conj, norm2 and the unit (Artin 11.3).
 *   involution  conj is Q-linear, conj(conj a) = a, conj(a b) = conj(b) conj(a) (an anti-automorphism); a + conj(a) is rational; a != conj(a) from level 1.
 *   norm        a conj(a) = conj(a) a = norm2(a) = the sum of the squares of the components, a rational > 0 for a != 0, and norm2(a b) = norm2(a) norm2(b)
 *               at levels 0..3 (the two-, four- and eight-square identities; Hurwitz: no other level has one). Re(a conj(b)) is a symmetric positive
 *               definite bilinear form (polarization).
 *   division    inv(a) = conj(a) / norm2(a) is a two-sided inverse for every a != 0, and a x = 0 only if a = 0 or x = 0. Q is ordered, so the norm is
 *               anisotropic: no floating point and no topology are needed for this. (Baez 1.1: for non-associative algebras "has inverses" and
 *               "has no zero divisors" are different properties; here both hold at every level of the tower.)
 *   centre      the centre is Q at levels 2 and 3 (every non-real basis element anticommutes with another one), and the whole algebra at levels 0 and 1.
 *   regular     x -> a x and x -> x b are Q-linear maps of Q^dim (lmat/rmat entries below), so the product is exact linear algebra over Q.
 *   rep.        Exactly (Baez 2.3): L_a^T L_a = norm2(a) I for every a; for imaginary a, L_a^T = -L_a and L_a^2 = -norm2(a) I; for imaginary a, b:
 *               L_a L_b + L_b L_a = -2 <a,b> I (a Clifford relation). The map a -> L_a is multiplicative at levels <= 2 and NOT at level 3.
 *   symmetry    Level 2: for q != 0 the map x -> q x q^-1 is an automorphism of H over Q that fixes the real part and the norm, composes as q -> C_q
 *               (a homomorphism whose kernel is the centre, the exact rational form of Artin's spin homomorphism SU2 -> SO3, Theorem 9.4.1, kernel {+-I}),
 *               and for imaginary u, v: u v = -(u.v) + u x v (Artin, Lemma 9.4.4). Level 3: Aut(O) = G2 (dimension 14, Baez 4.1); x -> u x u^-1 is
 *               well defined (flexibility) but is NOT an automorphism in general; the SO(4) inside G2 acts as (a, b) -> (p a p^-1, q b p^-1) for unit
 *               quaternions p, q; on imaginary octonions a x b = (ab - ba)/2 obeys |a x b|^2 + <a,b>^2 = |a|^2 |b|^2 and is orthogonal to a and b.
 *
 * Every operation either returns the exact answer or a status, never a rounded one (the Forbidden Zone is fail-closed):
 *   COS_E_RANGE    an exact intermediate rational does not fit cos_rat's 64-bit range
 *   COS_E_INVALID  level above COS_ALG_MAX_LEVEL, a NULL pointer, an operand that is not canonical (a bad rational, or a nonzero unused component),
 *                  or the inverse of zero
 * On error *out is left unchanged, and out may be the same object as an input. No heap, no floating point, no global state, no recursion, pure functions
 * of their arguments; the stack use is a few hundred bytes. The C++ side meets this header only through extern "C"; no STL is involved anywhere.
 *
 * Sources (project books read 2026-11-05; page and line facts are in docs/DAILY_INTUITION.md):
 *   J. Baez, "The Octonions", Bull. AMS 39 (2002): sections 1.1, 2.1 to 2.4, 4.1;  J. Baez, review of Conway and Smith, "On Quaternions and Octonions"
 *   (the project file conway_smith.pdf is this review, not the book): SU(2) as the unit quaternions, the Hurwitz units used as test generators;
 *   M. Artin, "Algebra" 2nd ed.: Definition 11.1.3 (the book restricts "ring" to commutative rings, so the non-commutative statements rest on the other
 *   sources), 11.3, 11.7, 9.3, 9.4, problem M.4;  MIT 18.755 (Etingof) section 6.2 and Exercise 3.9;  MIT 18.701 (Artin) lecture 29.
 *   NOT from a book: the exact-rational setting, the sign rule, the regular-representation bridge and the refusal rules; those rest on the tests.
 * C17, written for the Saem sovereign kernel. */
#ifndef COS_ALG_H
#define COS_ALG_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cos_rat.h"
#include "cos_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COS_ALG_MAX_LEVEL 3u
#define COS_ALG_MAX_DIM 8u

typedef struct CosAlg {
  CosRat c[COS_ALG_MAX_DIM];
} CosAlg;

uint32_t cos_alg_dim(uint32_t level);                         /* 1, 2, 4, 8 for level 0..3; 0 for any other level */
int cos_alg_basis_sign(uint32_t level, uint32_t i, uint32_t j); /* +1 or -1: e_i e_j = sign e_{i xor j}; 0 when level or an index is out of range */
bool cos_alg_valid(uint32_t level, const CosAlg *a);          /* canonical: used components are valid CosRat, unused ones are exactly 0 */

void cos_alg_zero(CosAlg *out);                               /* every component 0 (valid at every level) */
CosStatus cos_alg_one(uint32_t level, CosAlg *out);           /* e_0 */
CosStatus cos_alg_basis(uint32_t level, uint32_t i, CosAlg *out); /* e_i; i >= dim is COS_E_INVALID */
CosStatus cos_alg_from_ints(uint32_t level, const int64_t *v, size_t n, CosAlg *out); /* n <= dim integers; missing ones are 0 */

bool cos_alg_eq(uint32_t level, const CosAlg *a, const CosAlg *b);   /* exact; false if either is not canonical */
bool cos_alg_is_zero(uint32_t level, const CosAlg *a);

CosStatus cos_alg_add(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out);
CosStatus cos_alg_sub(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out);
CosStatus cos_alg_neg(uint32_t level, const CosAlg *a, CosAlg *out);
CosStatus cos_alg_scale(uint32_t level, CosRat s, const CosAlg *a, CosAlg *out);   /* s a for a rational s (it commutes with everything) */
CosStatus cos_alg_mul(uint32_t level, const CosAlg *a, const CosAlg *b, CosAlg *out); /* the Cayley-Dickson product, in this operand order */
CosStatus cos_alg_conj(uint32_t level, const CosAlg *a, CosAlg *out);
CosStatus cos_alg_norm2(uint32_t level, const CosAlg *a, CosRat *out);                /* a conj(a): the sum of the squares of the components */
CosStatus cos_alg_inv(uint32_t level, const CosAlg *a, CosAlg *out);

/* The natural inclusion of level `from` in level `to` (from <= to): the same components, the rest 0. It preserves +, -, *, conj and norm2. */
CosStatus cos_alg_embed(uint32_t from, uint32_t to, const CosAlg *a, CosAlg *out);

/* Entries of the Q-linear maps x -> a x and x -> x b on Q^dim (the regular representations), so that the product is a matrix-vector product:
 *   (a x)_row = sum over col of lmat(a)[row][col] x_col     with lmat(a)[row][col] = sign(row xor col, col) a_{row xor col}
 *   (x b)_row = sum over col of rmat(b)[row][col] x_col     with rmat(b)[row][col] = sign(col, row xor col) b_{row xor col}
 * row, col < dim, else COS_E_INVALID. */
CosStatus cos_alg_lmat_entry(uint32_t level, const CosAlg *a, uint32_t row, uint32_t col, CosRat *out);
CosStatus cos_alg_rmat_entry(uint32_t level, const CosAlg *b, uint32_t row, uint32_t col, CosRat *out);

#ifdef __cplusplus
}
#endif
#endif
