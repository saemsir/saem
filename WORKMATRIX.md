# WORKMATRIX: the Work-Time Engine (spec and status, 2026-11-06)

The user's TOP PRIORITY (2026-11-04 19:08 IST; the Creator Book `Vision Instructions for QCOS.pdf`): an n-dimensional quaternion/octonion matrix engine that explores algebraic geometry,
differential geometry and linear algebra, with axes vision, architecture, steps, problems, solutions, books, maths, physics, computation, concepts, equations, code (and more, added with time),
a LAST axis = time, relations and functions held in the matrices, and an advanced-calculus layer on top "to work at the right time in the right way", so that anyone, at any time, and QCOS itself,
can complete, upgrade and repair QCOS without being misled. "It is a very long work, do it bit by bit, slowly, wisely."
Rule for every line below: **verified** = a test or an oracle ran it; **planned** = a design no test has run. Nothing here is claimed beyond that.

## 1. The picture in one page

```
axes registry ──> nodes (axis, item) ──> relations (edges with algebra weights) ──> constraints (Ax = b)
      │                  │                          │                                   │
      │            state per node            incidence matrix A, weights C       projection / repair
      │          (exact CosAlg value)        K = A^T C A (graph Laplacian)       (Lagrange multipliers)
      v                  v                          v                                   v
 time axis (append-only, sealed, reversible) <── calculus layer (heat, wave/leapfrog, spectra) <── self-repair loop
                                                                                              detect → project → propose → Zone 0 → journal → verify
```
Everything is exact rational arithmetic in the tower Q, Q(i), H (quaternions), O (octonions). No float, no heap, no global state, no recursion. Refusal (COS_E_RANGE / COS_E_INVALID) is the only alternative to an exact answer.

## 2. Slice plan and honest status

| Slice | Content | Done when | Status |
|---|---|---|---|
| S1 `cos_alg` | exact Cayley-Dickson tower, levels 0..3, level 4+ refused | integrity contract (below) tested against an independent oracle; mutation self-test | **DONE, verified** (353,429 C checks; 19 oracle groups; 23 of 23 seeded faults caught; gate 69 of 69) |
| S1.5 `cos_rat` wide path | 128-bit intermediates (hand-written two-limb helpers, no `__int128`, no libgcc call) so add/sub/accumulate refuse only when the REDUCED result does not fit 64 bits; the gate today reports 167 of 138,265 sums refused though they fit | the 192,805-command Fraction cross-check shows 0 such refusals; object audit unchanged | planned, NEXT (it is also the user's 2026-11-06 directive: "124-bit common denominator scaling ... overflow caught at the accumulation phase") |
| S2 `cos_mat` | exact matrices over Q (and algebra-valued ones through the regular representation): RREF, rank, null space, column space, left null space (Strang's four subspaces), solve, inverse, projection | tests against Fraction RREF; the four subspaces satisfy dimension and orthogonality theorems exactly | planned |
| S3 relations | constraint matrix, tangent space, minimal-norm repair, exact multipliers, connection and holonomy, graph Laplacian | holonomy of a flat connection is exactly 1; repair lands on the constraint set; multipliers satisfy A^T lambda = residual direction | planned |
| S4 work-tensor registry | axes registry, nodes, edges, function-id table, append-only time axis, queries, journal/seal integration | static bounds by `static_assert`; relabel-invariance tests (section 6); nothing dynamic | planned |
| S5 calculus layer | grad = A, div = A^T, K = A^T C A, heat, wave with leapfrog, Kronecker sums for n axes, exact discrete energy, CFL as the discrete light cone | conserved quantities hold exactly over thousands of steps; reversibility exact | planned |
| S6 self-repair loop | detect (exact residuals, holonomy), project, propose, Zone 0 gate, journal, apply, verify; sealed core never repairable | seeded corruptions repaired or refused, every step journaled | planned |
| S7 content and frontends | real QCOS content in the axes; R (`R.h` .Call into the C kernel), Julia offline workbench, Python oracle | R and Julia are NOT installed in this workspace: planned only | planned |

## 3. Slice 1 (verified): what the tower guarantees

API and the full integrity contract live in `core/c17/cos_alg.h` (read it first); the proofs are `core/tests/c17_alg_test.c`, `tools/check_alg.py`, `tools/alg_oracle.py`.
- Levels: 0 = Q, 1 = Q(i), 2 = Hamilton quaternions over Q (division ring, centre Q), 3 = octonions over Q (alternative division algebra, centre Q). Level 4 and above are refused: the sedenions have zero divisors, (e1 + e10)(e4 - e15) = 0, and the norm stops being multiplicative (fails on 150 of 150 random pairs at levels 4 and 5).
- Exactly: ring axioms and the Q-algebra laws; the embedding of each level in the next is an injective unital homomorphism; conj is an anti-automorphism; a conj(a) = N(a) = sum of squares; N(ab) = N(a) N(b); inverse conj(a)/N(a) is two-sided and there are no zero divisors; octonions are alternative, flexible, Moufang, power-associative, Artin's theorem holds, and the associator witness (e1 e2) e4 = -e1 (e2 e4) shows they are not associative.
- Regular representation (the bridge to linear algebra): L_a^T L_a = N(a) I; imaginary a: L_a^2 = -N(a) I; Clifford relation L_a L_b + L_b L_a = -2 <a, b> I. a -> L_a is multiplicative up to level 2 only: for octonions operator composition is not the entrywise product.
- Symmetry: exact rational spin map x -> q x q^-1 (kernel = centre), 3 x 3 rational rotations R^T R = I, det 1; level 3: SO(4) inside G2, conjugation by a unit is NOT an automorphism, 7-D cross product identities.
- Overflow: every operation returns the exact answer or COS_E_RANGE with *out untouched (also when out aliases an input); proven on products whose terms fit but whose sum does not (2^62 + 2^62, a 124-bit common denominator).
- Independence of the oracle: the kernel multiplies by signed basis products from a four-case sign rule; the oracle multiplies by the pair rule on Python Fractions and shares no code. Baez's published mirror convention is carried to ours by the explicit isomorphism psi(a, b) = (psi a, conj psi b).

## 4. The axes, nodes, relations and functions (slice 4 design, planned)

- Axis registry: a `const` table of at most 16 axes `{id, name, kind, capacity}`; the first twelve are the user's (vision, architecture, steps, problems, solutions, books, maths, physics, computation, concepts, equations, code); slots remain for "other things added with time"; TIME is always the last axis and is append-only.
- Node = (axis, item) packed into a 16-bit id; per node an exact state value (a CosAlg at a per-axis level) and a status.
- Relation = an edge (src, dst, weight). The weight is an algebra element: a rational for a plain dependency, a unit quaternion or octonion when orientation (holonomy) matters.
- Function = a numbered entry in a `const` table of named exact operations (add, mul, conj, inv, norm2, rotate, project, holonomy, ...): cells refer to functions by id, never by pointer, so the table can be sealed, versioned and audited.
- Memory: one caller-provided arena, `alignas(64)`, sizes fixed by `static_assert` (placeholders to be fixed in slice 4: 256 nodes, 1024 edges); a snapshot ring of three states for the leapfrog; history lives in the sealed journal, not in RAM.
- Meaning is not representation (the axiom MEANING != REPRESENTATION != EXECUTION != HARDWARE): ids are labels; no quantity may depend on an id's numeric value (section 6).

## 5. Linear algebra and the constraint variety (slices 2 and 3, planned)

- Exact elimination over Q: RREF, rank, null space, Strang's four subspaces. Algebra-valued matrices are Q-linear operators on Q^(d n) through the regular representation (verified), so the same elimination serves all four levels. The kernel working limit is 32 rows (COS_WORKING_LIMIT); matrices live in caller storage because a function may use at most 2 KB of stack (-Wstack-usage=2048).
- Constraints are Lagrange's "equations de condition" (Mecanique analytique, Statique II arts. 10 to 12): the state x must satisfy A x = b. The independent coordinates are the null space of A (art. 12: "ou d'autres variables quelconques"). For a violated state the nearest consistent one is x' = x - A^T (A A^T)^-1 (A x - b), exactly in Q; lambda = (A A^T)^-1 (A x - b) are the Lagrange multipliers (Statique IV par. I, p. 74), the exact "price" of each broken invariant.
- Connection and holonomy: edge weights in the unit quaternions (or octonions) define parallel transport; around a closed loop the product must be exactly 1 for a flat (drift-free) system. Octonion products are not associative: every path is folded LEFT in edge-list order, and the engine states that rule rather than hiding it.
- Failure mode to design against: rational elimination grows fractions fast; with 64-bit storage the engine refuses (fail closed). Slice 1.5 widens intermediates so refusal happens only when the final reduced value does not fit.

## 6. What the physics directives become (honest labelling)

- Covariance (Einstein 1916 par. 3, "space-time coincidences ... coordinates serve only an easy description"): the testable rule is RELABEL-INVARIANCE. Permuting node ids or axis order must leave every invariant (norms, residuals, holonomies, energies, canonical hash) unchanged. This is also the rule for `sir`: no address, offset or register name may enter the meaning of an IR node. Planned test in slices 4 and sir.
- Invariant interval: what we have is the EXACT EUCLIDEAN norm N(ab) = N(a) N(b) (verified). The spacetime interval is indefinite (t^2 - |x|^2) and the Cayley-Dickson norm is not. If the time axis needs a Lorentz form, it is a separate exact quadratic form with exact rational boosts (beta = 2pq/(p^2 + q^2) gives a rational gamma); planned in slice 5, not claimed now.
- Variational action (Einstein 1916 par. 15, Hamilton's principle with variations vanishing at the limits of the finite integration space; Lagrange; Maupertuis): the time axis is a discrete action principle with fixed endpoints. Leapfrog for M u'' + K u = f is exactly time-reversible and conserves, exactly, E_n = |U_(n+1) - U_n|^2 / h^2 + U_(n+1)^T K U_n for symmetric K (derived, to be tested in slice 5).
- "Curvature routing" (the directive's negative-pressure idea, trace-free field equations): made concrete as a pressure-weighted metric. Edge weight w_e = 1 + kappa p(e) with p the occupancy of the arena behind e; routes are least-weight paths (geodesics of the deformed metric), so traffic bends away from full arenas. It is an analogy made testable, not a solution of the Einstein equations. The 1917 and 1919 papers are not in the project books.
- Closed finite universe: already the zero-heap rule (a static arena). Discrete light cone: the CFL ratio r <= 1; the domain of dependence of a node after n steps is the graph-distance ball of radius n (slice 5).
- Amorphous medium (Beal): the node graph IS the medium; fields are node values; operators are local (neighbour sums through K); homeostasis is the repair loop. Dirac: non-commutative operator ordering is the quaternion/octonion product order, fixed and tested. Fourier/NTT: spectral decomposition of K in slice 5 (the Fourier book is not read yet).

## 7. Self-repair and governance (slice 6, planned)

Loop: detect (exact residual A x - b, holonomy != 1, hash mismatch) -> project (minimal exact correction) -> propose (a plan is data, never executed by the engine itself) -> Zone 0 gate (constitution; fail closed in the Forbidden Zone) -> journal (hash-chained, receipt before action: "no receipt, no action") -> apply -> verify (re-check residuals). The sealed core (constitution tables, keys, the engine's own contract tests) is not repairable by this loop: one changed byte freezes the system (already implemented and tested in `cos_core`). QCOS may repair, upgrade or evolve everything outside the seal, and only through this loop; the Qur'an and the Sunnah rank above any order, including the creator's (Vision Instructions), and that rule lives in Zone 0, not here.

## 8. Frontends and tools

C17 is the single source of truth. Python (`tools/alg_oracle.py`, `tools/check_alg.py`) is the independent oracle and test automation only. R: the C functions are called through `.Call` with `R.h` wrappers to plot manifold states and cross-check numbers; Julia: offline workbench for hypercube and spectral prototypes, translated to static C before it enters the backend. Neither R nor Julia exists in this workspace, so neither is built or tested yet. No STL anywhere; the C++ side meets the headers through `extern "C"`.

## 9. Decisions and open questions

1. DECIDED (mine, reversible): the tower stops at the octonions; sedenions would break N(ab) = N(a) N(b) and introduce zero divisors.
2. DECIDED: exact or refuse. A rounded value is never returned.
3. OPEN, not blocking: a 128-bit wide path in `cos_rat` (slice 1.5) changes the documented refusal limits of add and sub; I propose to do it next because slices 2 to 5 need it.
4. OPEN: the repository path name. The directives say `core/saem/`; the sovereign kernel lives in `core/c17/`. I keep `core/c17/` (no rename) until you say otherwise.
5. OPEN: `check_sir` is named in the directives; SIR steps 3 and 4 do not exist yet, so neither does it.

## 10. Sources (all project books; see docs/DAILY_INTUITION.md, entry 2026-11-04 to 2026-11-06, for sections and page numbers)

Baez, The Octonions; Baez's review of Conway and Smith; Artin, Algebra 2nd ed.; MIT 18.701 (Artin) lecture 29; MIT 18.755 (Etingof) 6.2; Hamilton, Lectures on Quaternions (1853); Lagrange, Mecanique analytique;
Einstein 1916 in The Principle of Relativity; Strang, Computational Science and Engineering; Beal, the amorphous-medium papers (AIM-2003-xxx, AIM-2005-001, amorphous-medium-UPP-LNCS); Hillis and the Connection Machine papers; the Creator Book `Vision Instructions for QCOS.pdf`.
NOT in the project: MIT 18.703 and 18.745. NOT yet read: the Turing set, Fourier, MIT 18.03 c01, 8.04, 8.321, `computation_structures-mit.pdf`.
