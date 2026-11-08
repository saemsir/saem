# Work-Time Engine operating guide

This is the repeatable work procedure for continuing the Work-Time Engine
(WTE) across sessions and contributors. The slice definitions, dependencies,
and completion evidence in [WORKMATRIX.md](WORKMATRIX.md) are authoritative;
this guide explains how to execute them without skipping scope or overstating
progress.

## Resume protocol

1. Read [STANDING_ORDERS.md](../../STANDING_ORDERS.md), the current WTE
   [work matrix](WORKMATRIX.md), the latest
   [status handoff](../../saem-unified/docs/STATUS.md), and the last three
   entries of [DAILY_INTUITION.md](../../DAILY_INTUITION.md).
2. Read the corresponding engineering progress entry in
   [PROGRESS_c17.md](../../saem-unified/docs/PROGRESS_c17.md). Treat code and
   runnable tests as the evidence for implementation status.
3. Resume the first incomplete slice below. If a slice is in progress, finish
   or checkpoint that slice before starting another. Do not select work by
   convenience or jump ahead to a later slice.
4. Before implementation, select no more than three relevant books or
   documents. Record the exact sections, equation/invariant, useful code
   pattern, and failure mode in the daily log.

## Fixed slice order

| Order | Slice | Scope | Entry condition |
|---|---|---|---|
| 1 | S1 `cos_alg` | Exact Cayley-Dickson algebra levels 0-3; refuse level 4+ | Complete; do not reopen without a concrete failing test or changed requirement |
| 2 | S1.5 `cos_rat` | Exact wide rational add/subtract | Complete; do not reopen without a concrete failing test or changed requirement |
| 3 | S2 `cos_mat` | Exact rational matrices, RREF, rank, four subspaces, solve, inverse, projection | **Next** |
| 4 | S3 relations | Constraints, tangent space, exact repair/multipliers, connection and holonomy | S2 completion evidence recorded |
| 5 | S4 registry | Bounded axes/nodes/edges/functions and append-only time axis | S3 completion evidence recorded |
| 6 | S5 calculus | Discrete operators, heat/wave updates, spectra, energy and reversibility | S4 completion evidence recorded |
| 7 | S6 self-repair | Detect, project, propose, gate, journal, apply, verify | S5 completion evidence recorded |
| 8 | S7 content/frontends | Populate the WTE and add only verified frontend/oracle work | S6 completion evidence recorded |

After S7, use the separate backlog ordering in
[STATUS.md](../../saem-unified/docs/STATUS.md). The EdTech book is product
context, not a substitute for the mathematics sources or tests required by a
WTE slice.

## Per-slice work loop

### 1. Establish the contract

- Restate the single slice objective and its matrix exit condition.
- Read the existing public headers, implementation, tests, build/check
  scripts, and relevant status notes before editing.
- Turn each claimed invariant into an observable test. Record expected bounds,
  failure statuses, aliasing/output behavior, and whether exact refusal is
  required.
- Preserve the current kernel constraints in the source and build: C17,
  bounded caller-owned storage, no heap, globals, or recursion, stack usage
  within the existing limit, `COS_WORKING_LIMIT` respected, and exact rational
  results or explicit refusal. Confirm the current limits in code rather than
  copying an assumed value into a new implementation.

### 2. Implement one bounded change

- Keep the change inside the active slice and its directly necessary tests or
  documentation. Do not add a later slice's API "for convenience."
- Use established naming, status values, and fail-untouched output patterns.
- Keep independent test oracles independent in both representation and
  algorithm; do not duplicate the implementation in the oracle.
- Do not silently round, truncate, substitute a default, or convert an
  unsupported case into success.

### 3. Verify the exact claim

- Run the smallest relevant C test and independent oracle first.
- Run the repository's focused C17 checks for the changed component, including
  strict compiler/sanitizer or object-code checks when available.
- For mathematical claims, check the exact theorem or property named in the
  matrix (dimensions, orthogonality, constraint satisfaction, holonomy,
  energy, or reversibility), not merely that the code returns a value.
- Run `git diff --check`. Report unavailable tools, skipped checks, and
  environment failures separately; none count as passing evidence.
- Update the matrix status only when the stated exit condition has actually
  passed. Otherwise leave the slice planned/in progress and record the precise
  failing or blocked check.

### 4. Save a resumable checkpoint

At the end of each work item, even if the slice is unfinished:

- Append to `DAILY_INTUITION.md`: date, slice, sources/sections, invariants,
  code pattern, failure modes, files changed, exact tests and results, and the
  next action.
- Update `WORKMATRIX.md`, `saem-unified/docs/STATUS.md`, and
  `saem-unified/docs/PROGRESS_c17.md` consistently. Keep planned, implemented,
  tested, and environment-blocked claims distinct.
- State the next slice and its entry condition in the handoff. Preserve
  unrelated worktree changes; do not reset or discard them.
- Do not commit or push unless the user asks.

## Stop and decision rules

Stop within the current slice when its contract is unclear, a required source
is missing, a test contradicts the design, a bound cannot be met, or a proposed
change would alter externally visible behavior. Record the evidence and the
smallest decision needed; do not guess and do not advance to the next slice.

Treat design documents and books as input, not proof. In particular, the
Saem Global EdTech book contains several evolving and conflicting platform
proposals. Do not treat its sample code, security examples, AI-generated
patch/auto-commit workflows, or product targets as implemented or approved
kernel behavior. Never copy credential-like values into source, logs, or new
documentation.
