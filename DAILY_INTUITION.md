# Daily intuition log (newest first)

## 2026-11-08: Saem Physics channel site and staged EdTech architecture

Work item: organize the current website and supplied references into a truthful
step-by-step path toward the offline-first physics learning product.

### Sources actually inspected

- Workspace [README.md](README.md), [index.html](index.html), [script.js](script.js),
  and [style.css](style.css): the existing site is a static channel page with
  curated YouTube links, topic/playlist/lecture cards, admission, Telegram, and
  live-class links.
- [STANDING_ORDERS.md](STANDING_ORDERS.md), sections 1–3, and
  [INSTRUCTIONS_2026-11-06.md](INSTRUCTIONS_2026-11-06.md): the product target
  includes forecasting with past-paper evidence, a timed test machine, worked
  solutions, weak-topic feedback, offline use, and durable save/continuity.
- [saem-unified-status.md](saem-unified-status.md), LATEST and the EdTech
  product state: the project records a C++ EdTech engine, C17 kernel/math work,
  and reference interface plans; this workspace currently has only the packed
  source snapshot, not an extracted backend tree.
- `open-math-harness-main.zip`, its README and Cargo manifest: it is a Rust
  runtime for Open Math OS; the README says the complete runtime depends on that
  deployment environment, while a small example is locally self-contained.
  Lean checks proof candidates. This is not established as an offline physics
  solver for the product.
- `xv6-riscv-fall19-xv6-riscv-fall19.zip`, its README: this is a 2019 RISC-V
  teaching OS and requires a RISC-V toolchain and QEMU to run. It is reference
  material, not the Saem production backend.
- [saem-unified-bundle.py.txt](saem-unified-bundle.py.txt): its manifest lists
  216 SHA-256-checked files, including `core/c17`, the EdTech `core/src`, tests,
  and docs. The bundle was inspected, restored to [saem-unified/](saem-unified/),
  and all extracted files were independently compared with their manifest
  digests.

The two supplied EdTech PDFs were identified as 351 pages and 8 pages, but no
PDF text extractor is installed in this workspace. Their contents have not
been read; no book equation or recommendation is attributed to them here.

### Equations and invariants

- Proposed, not implemented: a transparent smoothed occurrence baseline
  `p(topic) = (sum_i w_i*y_i + alpha)/(sum_i w_i + 2*alpha)`, with declared
  weights, `alpha > 0`, and paper `i` held out during back-testing. Evaluate
  predictions with a proper score such as `mean((p_i-y_i)^2)` and report
  calibration and evidence coverage. This is a product-design starting point,
  not a claim that the forecast works.
- Keep original paper language/script and source provenance separate from
  normalized text. Keep the browser UI, local app, EdTech engine, QCOS gate,
  research tools, and future RISC-V target behind explicit interfaces.
- The archived Open Math and xv6 projects have deployment/scope constraints;
  their mere presence does not make them production-ready dependencies.

### Code pattern and failure modes

- Reuse the already-working root-level HTML/CSS/JS channel site as Stage 0;
  make publication instructions match its actual flat directory layout.
- Use versioned, bounded JSON/process interfaces between language runtimes
  before attempting shared-memory or FFI integration. Keep Julia, R, and
  Fortran in offline research/prototyping roles until outputs are independently
  validated.
- Failure modes: claiming a static site has a working forecast backend;
  treating Open Math Harness as a standalone offline solver despite its README;
  treating teaching xv6 as the product kernel; reporting predictions without
  past-only back-tests; or claiming PDF knowledge without extracting the PDFs.

### What changed

- Added the staged boundaries, language roles, forecast evaluation proposal,
  source inventory, and verification gates to
  [docs/SAEM_EDTECH_ARCHITECTURE.md](docs/SAEM_EDTECH_ARCHITECTURE.md).
- Corrected the root [README.md](README.md) to describe and publish the site
  that is actually in this repository.
- Added share-card metadata to [index.html](index.html); no backend behavior
  or third-party runtime dependency was added.
- Restored all 216 source files under [saem-unified/](saem-unified/) without
  changing the root website and added a local README and generated-file ignore
  rules.
- Corrected the recovered `Makefile` to skip Python unittest discovery only
  when no Python test files exist; it prints an explicit message rather than
  returning unittest's "no tests" exit code.

### Snapshot verification results

- `node --test reference/test/*.test.js`: 35/35 pass.
- `python3 tools/build.py cli` and the C++ differential harness: 325/325
  cases agree.
- `make test`: passes; 35 JavaScript tests and 325 C++ differential cases pass.
  The target explicitly reports that no Python unit tests exist and skips that
  suite. No Python test coverage is claimed.
- `make c17`: 51/56 groups pass. Full exit is blocked by missing `ld.lld`
  (RISC-V link audit) and four ThreadSanitizer runs failing at startup with
  `FATAL: ThreadSanitizer: unexpected memory mapping`. The rational and algebra
  oracles, RV32/RV64 ISA comparisons, strict C builds, and non-TSan C tests
  passed. Do not mask these infrastructure failures as green.

One entry per working day, written BEFORE the code it drives (rule in `docs/STANDING_ORDERS.md` section 2).
Each entry: books and sections read, equations and invariants, code patterns, failure modes, and what changed in our code.
"Verified" means we read it in the project copy or ran it; "planned" means a design that no test has run yet.

## 2026-11-04 to 2026-11-06: the Work-Time Engine, slice 1 (the exact Cayley-Dickson tower `cos_alg`)

Work item: the user's TOP PRIORITY (2026-11-04 19:08 IST, and in the Creator Book `Vision Instructions for QCOS.pdf`): an n-dimensional quaternion/octonion matrix engine
(algebraic geometry, differential geometry, linear algebra) with axes vision, architecture, steps, problems, solutions, books, maths, physics, computation, concepts, equations, code,
a last axis = time, relations and functions in the cells, and a calculus layer on top; so that anyone, and QCOS itself, can complete, upgrade and repair QCOS. Spec: `docs/WORKMATRIX.md`.
Books read (project copies; "verified" = read in the extracted text):
`octonions.pdf` (Baez, The Octonions, Bull. AMS 39, 2002) 1.1, 2.1 to 2.4, 4.1; `conway_smith.pdf` (this is Baez's REVIEW of Conway and Smith, not the book);
`ArtinAlgebra.pdf` (Artin, Algebra 2nd ed.) 9.3, 9.4, 11.1, 11.3, 11.7, M.4; `mit18_701f21_full_lec_new.pdf` lecture 29; `mit18_755_s24_lec_full.pdf` 6.2 and Ex 3.9;
`Gilbert Strang MIT Computational Science and Engineering (1).pdf` (graph Laplacian, Kronecker sums, leapfrog, saddle points; page numbers to be re-found when slice 5 starts);
and, uploaded 2026-11-05, read 2026-11-06: `cu31924062612043.pdf` = Hamilton, Lectures on Quaternions (1853), Preface p. [47] and Lecture VI art. 291;
`mcaniqueanalyt01lagr.pdf` = Lagrange, Mecanique analytique, Part I (Statique) section II arts. 10 to 12 (pp. 38 to 40) and section IV par. I (p. 74);
`einstein principle of relativity mit (1).pdf` = The Principle of Relativity, Einstein 1916 par. 3 (pp. 97 to 98) and par. 15 (p. 138).
NOT yet read (arrived after the reading, planned): Turing set (7 files), Fourier `analyticaltheory00fourrich`, MIT 18.03 c01, MIT 8.04 lecture notes, MIT 8.321 lecture 1, `computation_structures-mit.pdf`.
NOT in the project at all: MIT 18.703 and 18.745 (named in the request of 2026-11-05); the project has 18.701 (Artin) and 18.755 (Etingof) instead. Einstein's 1917 (closed universe) and 1919 (trace-free field equations) papers are not in the collection either: those two points of the directive rest on the directive text, not on a book.

### Equations and invariants

1. Doubling rule (Baez 2.2 up to the explicit isomorphism psi(a, b) = (psi a, conj psi b), which the oracle checks at levels 1 to 4): (a, b)(c, d) = (ac - conj(d) b, da + b conj(c)); conj(a, b) = (conj a, -b).
   Every basis product is a signed basis element, e_i e_j = sign(i, j) e_(i xor j); the sign follows from a four-case rule applied top bit to bottom bit (no recursion).
2. Norm: a conj(a) = sum of squares; N(ab) = N(a) N(b) at levels 0 to 3 and at no level above (Hurwitz). Hamilton found it for four squares in 1843 "not being acquainted with a theorem of Euler respecting sums of four squares" (Preface p. [47]).
   Measured here: it holds on every random pair at levels 0 to 3 and fails on 150 of 150 pairs at levels 4 and 5; the explicit sedenion zero divisor (e1 + e10)(e4 - e15) = 0 with both norms 2 is why the tower stops at the octonions.
3. Ring and division (Artin 11.1, 11.3, 11.7; 18.701 lecture 29): Q and Q(i) are fields (-1 is not a square in Q); the Hamilton quaternions over Q are a division ring with centre Q. CAUTION: Artin Definition 11.1.3 defines "ring" as commutative, so every non-commutative statement rests on Baez and the lectures.
4. Octonions (Baez 1.1, 2.2, eq. (1)): alternative, flexible, three Moufang identities, power-associative, Artin's theorem (two elements generate an associative subalgebra); for non-associative algebras "has inverses" and "has no zero divisors" are different properties; here both hold.
5. Regular representation (Baez 2.3): L_a^T L_a = N(a) I; imaginary a: L_a^T = -L_a and L_a^2 = -N(a) I; imaginary a, b: L_a L_b + L_b L_a = -2 <a, b> I. The map a -> L_a is multiplicative up to level 2 and NOT at level 3.
6. Symmetry: x -> q x q^-1 is an exact rational automorphism of the quaternions (Artin 9.4.1, kernel = centre; the 3 x 3 rotation matrix has R^T R = I and det 1); Hamilton's own product of two vectors, Preface p. [47]: (ix + jy + kz)(ix' + jy' + kz') = w'' + ix'' + jy'' + kz'' with w'' = -xx' - yy' - zz', i.e. u v = -(u.v) + u x v (Artin Lemma 9.4.4);
   Lecture VI art. 291: T(g r g^-1) = T g T r T g^-1 = T r by art. 188 (the tensor of a product), i.e. rotation keeps length. Octonions: Aut(O) = G2 (Baez 4.1); SO(4) inside G2 acts as (a, b) -> (p a p^-1, q b p^-1) (checked exactly); conjugation by a unit octonion is well defined but NOT an automorphism.
7. Lagrange (Statique II arts. 10 to 12): constraints are "equations de condition"; eliminate as many differentials as there are conditions so that the rest are independent, then set each coefficient to zero; any variables whatever may serve as coordinates ("ou d'autres variables quelconques").
   Section IV par. I (p. 74): the method of multipliers does the same without eliminating. For us (slice 3): the tangent space is the null space of the constraint matrix, and the multiplier of each constraint is its exact "price" in the repair.
8. Einstein 1916 par. 3: "All our space-time propositions amount to the determination of space-time coincidences ... The introduction of a system of co-ordinates serves no other purpose than an easy description of totality of such coincidences": only relabelling-invariant statements are physical.
   Par. 15: the Hamiltonian form delta(integral of H dtau) = 0 where "the variations vanish at the limits of the finite four-dimensional integration-space": a variational principle needs fixed endpoints, which our append-only, sealed time axis supplies.

### Code patterns

- Exact or refuse: every operation answers exactly or returns COS_E_RANGE and leaves *out alone; results are built in a local so out may alias an input.
- A sign rule instead of a table: three loop iterations, no recursion, no storage.
- An independent oracle that shares no idea with the kernel (the pair rule on Python Fractions) plus a stdin/stdout dump tool, three operand classes (small: must answer; mid and wide: exact-or-refused, never wrong, never touched).
- Mutation self-test: 23 seeded faults in cos_alg.c (a flipped sign, a reversed operand order, an ignored overflow, an aliasing bug ...) must each make the C suite fail.
- Theorems as tests: associator, alternativity, Moufang, norm composition, Clifford relation, rational spin map, 7-D cross product, SO(4) inside G2, witness of non-associativity.

### Failure modes the books warn about (and ones we hit)

- Octonions are not associative: a "product of three" needs an explicit bracketing; the engine will fold left in edge-list order and say so.
- Level 4 has zero divisors and no multiplicative norm: refused with COS_E_INVALID, never "extended".
- Our own: a green suite proves nothing until it can fail. The first mutation run found that no test overflowed in the ACCUMULATION step (every product fits, their sum does not): a kernel that swallowed that error would have passed. Fixed with two tests (numerator 2^62 + 2^62, and a 124-bit common denominator) and 23 of 23 faults are now caught. One mutant was equivalent (the inverse-of-zero check is redundant with cos_rat_div) and was replaced.
- Our own: a test bug (a 3 x 3 rotation block multiplied against a 4-component vector) looked like a kernel failure in 40 checks; the kernel was right.
- Hamilton: ijk = -1 and kji = +1; the operand order is part of the meaning. Reversing it gives a different algebra and is one of the seeded faults.

### What changed in our code because of this entry

- New: `core/c17/cos_alg.h`, `cos_alg.c` (levels 0 to 3, refuses 4 and above, no float, no heap, no globals, no recursion), `tools/alg_oracle.py`, `tools/check_alg.py`, `core/tests/c17_alg_test.c` (353,429 exact checks), `core/tests/c17_alg_dump.c`.
- `tools/check_c17.py` registers the alg test (O0, O2, O3, clang, ASan/UBSan) and the oracle cross-check: 69 of 69 checks, full run 1 min 32 s (49 before).
- Verified (ran): the whole integrity contract in `cos_alg.h`; kernel and oracle agree on 1920 products per level class with 534 of the wide ones correctly refused instead of rounded; both Baez conventions agree up to psi.
- Planned, not run: slices 2 to 7 (matrices, relations, registry, calculus, self-repair, content and R/Julia frontends); R and Julia are not installed here.

### Next reading (planned)

- Slice 2 (exact matrices, four subspaces, projection): Strang CSE chapters on elimination, the four subspaces and projection (re-find page numbers); Artin chapter 1 (matrices, row reduction).
- Slice 5 (calculus layer): Strang CSE (leapfrog, Kronecker sums); Fourier `analyticaltheory00fourrich` (heat equation, series); MIT 18.03 c01; 8.04 notes for commutators (the Dirac directive).
- Zone 0 and decidability: Turing 1936 and Petzold (discrete-state machines, "No capability, no access" as a state-machine property).

## 2026-11-01: the monitor core, atomics, PMP, the door

Work item: make the sovereign kernel fit the real "+1" core and design the door to Linux.
Books read, from the project copies (not only the repo clones): `riscv-privileged.pdf` (ratified 20250508) section 3.7 and chapter 6 (Smepmp);
`riscv-unprivileged.pdf` (ratified 20250508) sections 13.2, 13.3, 15.2, 16, 17.1 and appendix A.5;
`FU540-C000-v1.0.pdf` sections 1, 3.4 to 3.7, 4.4, 4.9, 5, 6, 9, 11, 12, 18, 25 (read 2026-10-31);
`linus.pdf` (Torvalds, Helsinki 1997) sections 1, 2, 3, 4.1, 4.3, 4.4, 5.1.

### Equations and invariants

1. NAPOT region of 2^k bytes (k >= 3) at base b (b a multiple of 2^k): `pmpaddr = (b | (2^(k-1) - 1)) >> 2`. For k = 3 this is `b >> 2`; the number of trailing ones is k - 3. (privileged 3.7.1.1, Table 19)
2. TOR: entry i matches y with `pmpaddr[i-1] <= y < pmpaddr[i]` (addresses shifted right by 2); entry 0 uses lower bound 0; if `pmpaddr[i-1] >= pmpaddr[i]` it matches nothing.
3. Priority (3.7.1.3): the lowest-numbered entry that matches ANY byte of an access decides, and it must match ALL bytes or the access fails. No match: M-mode succeeds; S/U fails when at least one entry exists.
   Consequence (derived, not stated in the book): a locked NAPOT catch-all with RWX = 000 at the highest index is default-deny for M-mode without Smepmp.
4. L bit (3.7.1.2): locks the entry (and `pmpaddr[i-1]` if the entry is TOR) until hart reset, and enforces the permissions on M-mode too. Setting L locks even when A = OFF.
   Granularity: write all ones to `pmpaddr0`, read back, G = index of the lowest set bit, grain = 2^(G+2) bytes.
5. Smepmp (chapter 6): CSR `mseccfg` at 0x747. MML (bit 0) and MMWP (bit 1) are sticky until PMP reset; RLB (bit 2) is locked once cleared with any L set.
   With MML: L = 1 means "M-mode only", L = 0 means "S/U only". LRWX = 0010 is a shared region that M-mode may read and write and S/U may only read.
   That is a hardware one-way channel: our RESPONSE ring (monitor writes, Linux reads). LRWX = 0011 is read/write for both: our REQUEST ring (Linux writes; the monitor treats it as hostile input anyway).
   After MML is set, an executable M-only rule cannot be added: set code regions before MML, or with RLB during boot.
6. Constrained LR/SC loop (unprivileged 13.3): at most 16 instructions in sequence; between LR and SC only base-I instructions with no load, store, backward jump, taken backward branch, JALR, FENCE or SYSTEM;
   the retry code may branch backwards; SC has the same address and size as the latest LR; the region must have the eventuality property.
   Guarantee: livelock-freedom only (not starvation-freedom). Our "lock-free on many harts" claim rests on this, so it must be checked from the object code.
7. CAS from LR/SC (Listing 2): `lr.w t0,(a0); bne t0,a1,fail; sc.w t0,a2,(a0); bnez t0,cas`, four instructions when inlined.
8. Ordering maps (A.5, Tables 89 to 92): acquire load = `l; fence r,rw`; release store = `fence rw,w; s` (C/C++), Linux uses `fence.tso; s`;
   fully ordered RMW = `amo.aqrl` or `lr.aq; op; sc.aqrl`; `smp_wmb = fence w,w`; `smp_rmb = fence r,r`; `dma_*mb` the same on coherent DMA.
   All memory operations are single-copy atomic; an aligned AMO is one operation that is both load and store; a misaligned access may be split into pieces of any size (17.1).
9. Zacas (chapter 15): `amocas.w/d/q` need the PMA level AMOCASW/D/Q on the region (15.2). Zabha (chapter 16) adds byte and halfword AMOs; without it a byte flag costs a word-sized LR/SC emulation.
   That is why our `atomic_flag` journal lock compiled to `lr.w.aq / sc.w`.
10. FU540 E51 (manual 3.4, 3.5, 25.2, 25.3, 25.6): no data cache; `lr` and `sc` raise an access exception on DTIM and other uncached regions; AMOs work on DTIM and the peripheral region;
    ROCK-3: acquire/release ordering of AMOs is wrong on the E51 (use `.aqrl`); ROCK-2: the high 24 address bits are sometimes ignored (never rely on a trap for a wild pointer);
    CCACHE-1: treat every uncorrectable L2 ECC failure as fatal (our FREEZE).
11. FU540 PMP is the v1.10 kind: 8 entries per hart, no `mseccfg`. Locked entries are the only way to bind M-mode. DMA masters (Ethernet GEM, PDMA) are NOT checked by PMP and there is no IOMMU:
    a compromised Linux that can program them can write any physical memory. Custom silicon needs an IOPMP (planned).
12. Torvalds' thesis: three design goals simplicity, efficiency, compatibility, later portability (sections 2, 6); the virtual machine layer so common code never sees hardware (1, 2.4);
    fixed-size types and explicit byte order and alignment (4.1); the Alpha port broke code that used bytes as atomic flags (5.1); lock-free structures named as the alternative to locks (4.4.1);
    MPP: one OS image per node, distribution by message passing in user space, nodes need not run the same OS or hardware (4.4.2); Linux running on top of another kernel (Mach, L4) is an established design (1).

### Code patterns

- Single producer, single consumer ring: write the slot, store-release the tail; consumer load-acquires the tail, reads the slot, store-releases the head. Free-running counters, power-of-two capacity. No read-modify-write needed, so it works on the E51.
- Single-hart profile: relaxed load, compare, relaxed store. No `lr`, `sc`, `amo` or `fence` instruction at all.
- One header (`cos_atomic.h`) is the only place the kernel touches atomics, so a profile is one switch, not an edit of every file.

### Failure modes the books warn about

- A byte used as an atomic flag (thesis 5.1; Zabha rationale).
- An unconstrained LR/SC loop may never succeed on some implementations (13.3).
- A misaligned store may become partly visible even though another part faults on PMP (3.7.1.3).
- PMP does not see DMA (FU540, no IOMMU).
- A PMP rule that is not locked is ignored by M-mode firmware (v1.10).

### What changed in our code because of this entry

- `core/c17/cos_atomic.h` (new): the only file that touches an atomic. Profile `COS_SINGLE_HART=1` is relaxed load, compute, relaxed store (equation 12's Alpha lesson and equation 10's E51 facts); profile 0 is the old C11 code, memory orders unchanged.
  Proof: default objects byte-identical (8 of 9), the journal object changed on purpose; one-hart objects hold no `lr`, `sc`, `amo`, `fence`.
- `cos_journal.c`: the lock is a 32-bit word with `exchange` (thesis 5.1: never a byte as an atomic flag; Zabha rationale, equation 9). 27 to 18 atomic instructions, 2816 to 2662 bytes. In the one-hart profile a re-entrant call returns the new `COS_E_BUSY` (21).
- `tools/check_riscv.py`: constrained-loop verifier from equation 6 (7 loops, longest 4 of 16; the checker is tested against 13 seeded violations); `-mstrict-align` pinned (FU540: misaligned accesses trap); one-hart object set and image.
  A surprise from this reading-and-checking loop: a linked image without `.riscv.attributes` shows A, M and C instructions as `<unknown>`; the checker now passes `--mattr`.
- `tools/check_c17.py`: atomics-only-through-the-header rule; every source built in both profiles under gcc and clang; one-hart test variants (`sh-O2`, `sh-clang-O2`, `sh-asan`); 33 checks, full run green.
- Tests: threads replaced by a re-entry test (journal) and sequential workloads (pool, core) in the one-hart build.
- Not changed, deliberately: PMP, Smepmp, the door and the OTP counter are designs only (`docs/RISCV_QCOS.md` sections 3 to 5); the lock's spin loop stays a plain test-and-set (a test-and-test-and-set refinement needs `cache & structure.pdf` read first).

### Next reading (planned)

- THE ASSEMBLY LAYER (the user's request of 2026-11-01 22:25 and the updated project instructions): `QCOS Compiler by SaemLabs.PDF` (their baseline: direct binary emitter, Zone 0 on register nodes, exporter and boot vector, CPU simulator),
  `HandP_RISCV(1..3)` (instruction formats and encodings), `riscv-unprivileged.pdf` (base ISA encodings), and the machine-readable `riscv-opcodes`. Also new in the project: `Digital_Design_and_Computer_Architecture_RISC (Sarah_Harris)`,
  `students digital guide - mit computer structure`, `Introduction to Assembly Language Programming`, `Programming_from_the_Ground_Up` (x86 assembly: useful for what an assembler is and does, not for RISC-V encodings).
- SIR parsing (strict JSON into a static node arena): `mit6_042js15_textbook.pdf` (recursive data types, structural induction), `Juhasz Modern C in embedded systems.pdf` (static memory patterns).
- Journal and receipts: `Principles of Computer System Design 2 mit.pdf` (all-or-nothing and before-or-after atomicity).
- Lock spin and false sharing: `cache & structure.pdf`, `HW Performance software system mit.pdf` (before changing the journal lock to test-and-test-and-set).
