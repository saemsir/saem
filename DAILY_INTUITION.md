# Daily intuition log (newest first)

## 2026-11-08 (continued): first exam desk and dynamic channel updates

Work item: move the static channel front door toward the SAEM Global EdTech
Stage 0/1 experience while keeping prediction claims tied to evidence.

- **Knowledge reviewed:** the staged
  [SAEM EdTech architecture](docs/SAEM_EDTECH_ARCHITECTURE.md) and its
  [full-book reading notes](books/edtech/README.md), especially the product
  direction, learning/forecast flow, provenance requirements, and separation
  between a static website and the future offline application. The book is an
  evolving proposal; its sample implementation claims were not treated as
  verified requirements.
- **Testing model:** added an original eight-question, 12-minute sample with a
  visible `+1 / 0` practice marking rule, answer navigation, timeout,
  deterministic scoring, topic breakdown, and worked review. This is not a
  teacher-reviewed JEE/NEET/board mock. Answers exist only in page memory.
- **Forecast intuition and equation:** for `n` cited, reviewed papers before
  target year `T`, and `k` papers tagged with a topic, the browser experiment
  uses the uniform Beta prior `p = (k + 1)/(n + 2)`. It requires at least five
  papers across three earlier years, filters to one exam series and language,
  and records the sources used. It is an exploratory frequency—not a chance
  that an exam question will appear. There is no curated paper archive or
  past-only calibration/back-test yet; labels and citations in imported files
  are user-supplied and not independently verified. Files are read in-browser,
  not uploaded or saved.
- **Channel updates:** video cards now work as dated upload posts with an
  optional bounded feed description. A student click activates an inline
  `youtube-nocookie.com` embed; no player is loaded until requested. A separate
  user-activated channel live-stream player is available if a public broadcast
  is running. The Pages workflow snapshots the public YouTube Atom upload feed
  hourly, and an open page checks the snapshot every 15 minutes. RSS does not
  expose Community posts or live-stream status, so the interface does not
  claim automatic live detection.
- **Changed:** `index.html`, `style.css`, `script.js`,
  `student-studio.js`, the student/feed tests, the channel-feed parser, Pages
  schedule, `README.md`, this log, the EdTech architecture, and this status
  handoff.
- **Verification:** the student-model tests cover all eight answers,
  unanswered/invalid scoring, cited/reviewed history validation, minimum
  coverage, language/series selection, smoothing, and exclusion of target-year
  papers. Six channel-feed parser tests cover valid metadata, channel
  validation, malformed IDs, bounded snapshots, and bounded descriptions.
  JavaScript syntax, 84 unique HTML IDs, 18 section links, and the live YouTube
  feed pass. Pages deployment `42500311464` succeeded; the published homepage,
  student module, site script, stylesheet, and 15-item feed return HTTP 200.
  There is no local browser installed for rendered visual QA.
- **Next:** gather legally usable, board/year/language-labelled papers and have
  a teacher review the questions and topic tags; only then add a curated,
  versioned dataset and past-only back-tests. Continue the kernel separately
  at S3 under its saved slice order.

## 2026-11-08 (continued): student practice and repository capability map

Work item: turn the static channel front door into a more useful, honest
student workflow while keeping future EdTech services clearly marked as
planned.

- **Knowledge reviewed:** the Saem Global EdTech reading notes, especially
  pp. 58–62 (calm visual explanation and low-end performance) and
  pp. 151–177 (learning, assessment, and pedagogy). Applied the recorded
  principles rather than copying the book's competing implementation
  proposals. The public 3Blue1Brown website was reviewed for general visual
  learning principles only; its distinctive artwork, layouts, and expression
  are not reproduced.
- **Learning model:** for constant acceleration,
  `x = ut + 1/2 at^2` and `v = u + at`. The guided check asks students to
  estimate, select an answer, and then read the worked reasoning. It includes
  the graph invariant `slope of x(t) = v(t)` and a negative-acceleration
  example that distinguishes acceleration direction from velocity direction.
- **Workflow and boundaries:** added a three-question practice room with
  keyboard-accessible radio choices, answer-specific explanations, progress,
  resume, and reset. Only a count of completed questions is stored in
  `localStorage`; no identity or responses are sent to a server. Added a
  capability map separating the deployed site, tested-but-not-connected C17
  and C++ source, and future offline/mock-test/forecast plans. These labels
  are intentionally explicit so the site does not promise a web API that does
  not exist.
- **Changed:** `index.html`, `style.css`, `script.js`,
  `student-studio.js`, `tools/test_student_studio.js`, the Pages workflow,
  `README.md`, `docs/SAEM_EDTECH_ARCHITECTURE.md`, this log, and
  `saem-unified-status.md`.
- **Verification:** student-model tests cover kinematics equations, invalid
  time/non-finite values, all three answer keys, and progress save/restore
  validation. The five YouTube feed tests and live feed generation pass.
  JavaScript syntax, 54 unique HTML IDs, 16 section links, deployment asset
  inclusion, and `git diff --check` pass. No local browser is installed, so a
  rendered visual QA pass is not available here.
- **Next:** verify the fresh Pages deployment and its new student interface;
  continue S3 only after reading its queued project-book sources.

## 2026-11-08 (continued): original visual learning design and channel updates

Work item: improve the Saem Physics homepage and keep recent public channel
uploads visible without introducing credentials or a runtime backend.

- **Reference reviewed:** the public 3Blue1Brown homepage at
  [3blue1brown.com](https://www.3blue1brown.com/), focusing on broad
  visual-learning principles: topic-led discovery, visual explanations, and
  concise paths into lessons. Its layout, brand, copy, and artwork were not
  copied. Saem's implementation uses its own warm-paper palette, field-note
  treatment, and mechanics diagram.
- **Equation/visual model:** the new hero diagram labels position `r(t)` and
  velocity `v(t) = dr/dt` on a particle path; the existing interactive lab
  continues to explore `x(t) = ut + 1/2 at^2`. The picture is an invitation to
  connect a geometric representation with a physical quantity, not a claim
  that an illustration replaces a derivation.
- **Code/data flow:** resolve the public channel handle to channel ID
  `UCJlNb8kQgLWqb65htic2CkQ`; read YouTube's public Atom feed during the Pages
  workflow; validate its channel and 11-character video IDs; serialize at most
  15 recent items as JSON; render six with DOM `textContent` and canonical
  YouTube watch URLs and lazy-loaded channel thumbnails. The workflow refreshes
  on pushes, manual dispatch, and every six hours. The page links to the full
  channel archive because the public feed is a recent-upload feed, not a
  complete channel export.
- **Failure/security limits:** no API key is stored or exposed. A malformed,
  empty, or unavailable feed fails the new deployment and leaves the previous
  Pages deployment intact. Browser rendering uses text nodes rather than
  inserting feed titles as HTML; if JSON is unavailable, the page displays a
  direct channel-archive link. GitHub Pages remains static.
- **Changed:** `index.html`, `style.css`, `script.js`,
  `.github/workflows/deploy-pages.yml`, `tools/fetch_channel_feed.py`, and
  `tools/test_fetch_channel_feed.py`; refreshed the README and status handoff.
- **Verification:** five parser tests pass for valid data, both channel-ID
  forms, channel mismatch, invalid IDs, and the 15-item bound. The live feed
  yields 15 valid uploads. JavaScript syntax, unique HTML IDs, fragment links,
  and `git diff --check` pass. Pages deployment `42497785994` succeeded; the
  live homepage, CSS, JavaScript, and channel JSON return HTTP 200. No local
  visual browser is installed; the SVG is responsive and labelled.
- **Next:** observe the scheduled refresh on its next run, then resume S3 after
  reading its project-book references. The full channel archive remains
  available directly from YouTube.

## 2026-11-08 (continued): exact solve, inverse, and column-space projection

Work item: complete the rational matrix API in Work-Time Engine slice S2
without extending the claim to algebra-valued matrices.

- **Books read:** revisited Strang, *Computational Science and Engineering*,
  pp. 36–38 for pivot columns and the four fundamental subspaces; read
  *Projection matrices and least squares*, pp. 1–2 for
  `P = A(A^T A)^-1 A^T`, `I-P` projecting onto `N(A^T)`, and the normal
  equations `A^T A x = A^T b`. MIT 6.042J p. 359 remains the relation-matrix
  context from the previous S2 entry.
- **Equation/invariant:** for a possibly rank-deficient `A`, choose independent
  pivot columns `B`; compute `P = B(B^T B)^-1 B^T` so the Gram matrix is
  invertible. Verify `P^T=P`, `P^2=P`, `Pb=b` for `b` in `C(A)`, and
  `(I-P)b=0` for `b` in `N(A^T)`. Solving uses exact Gauss-Jordan elimination;
  inconsistent systems refuse, while a consistent underdetermined system
  returns the particular solution with free variables zero.
- **Code pattern:** added caller-storage `cos_mat_solve`,
  `cos_mat_inverse`, and `cos_mat_projection`. Inverse columns and projection
  results are staged and copied to the output only on success. Projection
  extracts original pivot columns, forms their exact Gram matrix, and solves
  one small system per output column. The columnspace basis capacity is
  `rows*min(rows,cols)`, its tight maximum size.
- **Failure modes:** exact rational growth may exceed the 64-bit representation;
  preserve outputs and return the arithmetic error instead of rounding. Also
  reject inconsistent solves, singular inverses, undersized storage, and
  aliased buffers; the zero matrix projects to zero.
- **Changed:** `saem-unified/core/c17/cos_mat.{h,c}`,
  `saem-unified/core/tests/c17_mat_test.c`,
  `saem-unified/core/tests/c17_mat_solve_dump.c`,
  `saem-unified/core/tests/c17_mat_projection_dump.c`, and
  `saem-unified/tools/check_mat.py`; updated the S2 work matrix, status,
  progress handoff, and README.
- **Verification:** 183 C checks pass at GCC O0/O2/O3, Clang O2, and
  ASan/UBSan. The independent Fraction oracle matches 503 RREF matrices and
  all four basis sets, 250 systems, 100 inverses, and 150 projections.
  `python3 tools/check_c17.py --quick` reports 60/61: the only failure is the
  pre-existing RISC-V link audit, blocked by unavailable `ld.lld`. S2's
  rational API is complete; algebra-valued matrices remain future work. Next:
  read the queued S3 references before implementing relations and constraints.

## 2026-11-08 (continued): exact matrix RREF, rank, and four subspaces

Work item: continue Work-Time Engine slice S2 with exact bases for the four
fundamental subspaces; keep solve, inverse, and projection explicitly
unfinished.

- **Books read:** Strang, *Computational Science and Engineering*, pp. 36-38
  (four fundamental subspaces; `rank r`, dimensions `r`, `n-r`, `m-r`; basis
  independence and orthogonality); *Projection matrices and least squares*,
  pp. 1-2 (`P = A(A^T A)^-1 A^T`, the column-space/left-nullspace split, and
  `A^T A x = A^T b`); MIT 6.042J, p. 359 (relation matrices and Boolean
  matrix composition). The projection formula remains unimplemented.
- **Equation/invariant:** for `A` of shape `m x n` and rank `r`,
  `dim C(A) = dim C(A^T) = r`, `dim N(A) = n-r`, and
  `dim N(A^T) = m-r`; `C(A^T) perp N(A)` and `C(A) perp N(A^T)`.
  Exact Gauss-Jordan steps preserve the solution set of `Ax=b`, and pivot
  columns select a basis for `C(A)`.
- **Code pattern:** added caller-owned C17 matrix storage bounded by
  `COS_WORKING_LIMIT`, separate reduction/scratch buffers, exact `CosRat`
  row operations, and commit-on-success for RREF and basis outputs. Nullspace
  bases set free coordinates and solve pivot coordinates from RREF; rowspace
  uses nonzero RREF rows; columnspace uses original pivot columns; left
  nullspace applies the same construction to `A^T`. A Python `Fraction`
  elimination independently constructs and checks all four bases.
- **Failure modes:** rational elimination can exceed the kernel's 64-bit
  representation even when its input is valid; refuse with the arithmetic
  status rather than round. Invalid inputs and arithmetic refusal leave the
  original matrix and metadata unchanged. Tests also cover row/column
  limits, insufficient buffers, and overlapping storage.
- **Changed:** added four basis APIs in `core/c17/cos_mat.{h,c}`,
  `core/tests/c17_mat_test.c`, `core/tests/c17_mat_spaces_dump.c`, and extended
  `tools/check_mat.py`; the C suite and Fraction oracle are integrated into
  `tools/check_c17.py`. Updated the work matrix, status handoff, progress log,
  and this README.
- **Verification:** 45 C checks passed at gcc O0/O2/O3, clang O2, and
  ASan/UBSan; the independent Fraction oracle matched 503/503 RREF matrices
  and all four basis sets. `python3 tools/check_c17.py --quick` reported
  57/58; the sole failure is the existing RISC-V link audit, blocked because
  `ld.lld` is unavailable. Solve, inverse, projection, and their exact
  acceptance checks remain within S2.

## 2026-11-08: three parallel delivery lanes

Work item: save an executable order for frontend development, backend
development, and QCOS kernel development while keeping product and kernel
boundaries explicit.

- **Sources used:** the existing full-book review of
  [Saem Global EdTech.pdf](books/edtech/Saem%20Global%20EdTech.pdf) and its
  source notes in [books/edtech/README.md](books/edtech/README.md); the staged
  system boundaries in
  [SAEM_EDTECH_ARCHITECTURE.md](docs/SAEM_EDTECH_ARCHITECTURE.md); and the
  current [Work-Time Engine matrix](docs/qcos/WORKMATRIX.md) and
  [operating guide](docs/qcos/WORK_TIME_ENGINE_GUIDE.md). No additional PDF
  pages were reviewed for this documentation-only item.
- **Equation/invariant:** retain the existing visual lesson's
  `x(t) = ut + 1/2 at^2` as a frontend example, not a forecast formula.
  Kernel matrices remain exact over Q and must pass independent Fraction
  checks; the planned S2 exit conditions are unchanged.
- **Code/interface pattern:** keep HTML/CSS/JavaScript presentation, the local
  EdTech workflow and C++17 domain engine, and the C17 QCOS kernel in separate
  boundaries. Connect components later through a versioned, tested contract;
  start the kernel at S2 and preserve the fixed slice order.
- **Failure modes:** do not treat book proposals or performance targets as
  measured results; do not imply GitHub Pages runs a backend; do not merge
  kernel and customer-facing responsibilities; do not start dependent kernel
  slices before S2 passes.
- **Changed:** added the dated three-lane update and owner actions to
  [README.md](README.md), and saved the lane order and integration rules in
  [WORKMATRIX.md](docs/qcos/WORKMATRIX.md). No runtime code changed.
- **Verification:** documentation review and `git diff --check`; no code tests
  are applicable. Next: read the queued matrix references and begin S2
  `cos_mat`; separately choose the first learner cohort and paper-data
  sources for the product work.

## 2026-11-08 (continued): complete EdTech book review and WTE continuity guide

Work item: review the complete Saem Global EdTech book and make the Work-Time
Engine resumable slice by slice, with explicit scope and verification gates.

### Full-book review record

- Read [Saem Global EdTech.pdf](books/edtech/Saem%20Global%20EdTech.pdf)
  beyond the earlier targeted pages 58-61. The authoritative PDF page tree
  and local parser agree on 686 pages. The extracted text covers all 686 pages
  (91,466 words), with no empty pages or extraction errors. The review covered
  the book's evolving system architecture (pp. 1-22 and 180-611), Saem Physics
  product/platform proposals (pp. 23-150), learning, assessment, forecasting,
  and visual pedagogy (pp. 151-180), and model/mobile implementation material
  (pp. 612-686). Code samples and third-party/market claims were not
  independently validated.
- **Equation/invariants:** the architecture names A*'s `f(n) = g(n) + h(n)`
  (p. 2), but does not establish heuristic assumptions for a WTE use. Its
  recurring system boundary is protected core rules versus adaptable content;
  other stated constraints are offline use, bounded resources, and visual
  clarity without interface noise (pp. 1-4, 58-62, 151-177, 201-205,
  265-269).
- **Product design patterns:** render math clearly; keep the active learning
  task visually central; support curated question metadata, provenance, and
  teacher authoring/review; degrade expensive visual effects to a static
  representation when a measured performance budget requires it (pp. 58-62,
  89-90, 151-162). The proposed 45 FPS graphics threshold is a book target,
  not a measurement made by this static site.
- **Code pattern retained, not copied:** keep generated candidates behind
  explicit validation, separate local/offline learning data from optional
  synchronization, and use bounded user-controlled visuals. No book code was
  imported into the kernel or site in this work item.
- **Failure modes:** the compilation contains conflicting stacks and phased
  proposals (static HTML, Next.js/React Native, cloud services, Python/FastAPI,
  Kivy, and pure C); generated code and unattended auto-commit examples are
  not safe implementation instructions. Symbolic checks do not prove an
  explanation or forecast correct, and cryptographic hashes do not by
  themselves make a phone core tamper-proof (pp. 58-59, 95-103, 151-162,
  201-205, 612-686). The PDF includes credential-like examples; none are
  reproduced here.
- **WTE boundary:** the book's discussion of matrix views and four fundamental
  subspaces (p. 165) is pedagogical context only. S2 design and acceptance
  remain governed by the WTE matrix and its exact Fraction-oracle tests.

### What changed

- Added [WORK_TIME_ENGINE_GUIDE.md](docs/qcos/WORK_TIME_ENGINE_GUIDE.md) with
  the fixed S1, S1.5, S2, S3, S4, S5, S6, S7 order, resume protocol,
  per-slice entry and exit gates, stop rules, and checkpoint requirements.
  Linked it from the
  matrix and project indexes; made the matrix explicitly authoritative.
- Updated the EdTech book index, staged architecture, and project handoffs to
  record the full-document review without treating its proposals as deployed
  or verified code. Marked the bundled duplicate WTE matrix as a historical
  snapshot so its stale S1.5 status cannot override the active matrix. The
  companion EdTech book remains unread.
- No WTE implementation was started. The next implementation slice remains
  S2 `cos_mat`; first read the queued matrix/projection references as recorded
  in the WTE matrix.

## 2026-11-08 (continued): book library and visual physics lab

Work item: organize the supplied references by their primary use, resume the
Saem Physics visual-learning website, and read the relevant parts of the Saem
Global EdTech book before changing the experience.

### Reading and design notes

- Read the visual-learning, UI, and performance discussion in
  [Saem Global EdTech.pdf](books/edtech/Saem%20Global%20EdTech.pdf), PDF pages
  58-61. The local PDF page tree contains 686 pages. The supplied companion
  [Research and Codespaces Build Book](books/edtech/Saem_EdTech_Research_and_Codespaces_Build_Book.docx.PDF)
  remains unread; this is a targeted review, not a claim that all 686 pages
  were read.
- **Invariant/design rule:** a visual explanation should help expose a
  mathematical relationship without burying it in interface noise. The book
  also identifies low-end device performance as a constraint and proposes a
  static vector fallback below 45 FPS. That threshold is its proposal; this
  static site does not measure FPS.
- **Code pattern applied:** one bounded, original 2D Canvas model with
  explicit Play/Pause, time and acceleration controls, position/velocity/
  acceleration annotations, and the graph of
  `x(t) = ut + 1/2 at^2`. No GPU particle effect or autoplay is needed for this
  first explainer; the existing site colors and typography remain distinct.
- **Failure mode avoided:** excessive gamification/clutter and high-cost
  animation that can stutter or drain a phone battery. The new interaction is
  user-started, keyboard-operable through native controls, and has a static
  initial frame. A measured adaptive fallback is deferred until there is a
  performance case for it.
- The PDF also contains a credential-like value in an example configuration.
  It was not copied into code or notes. The original book is preserved
  unchanged; if the value is real, it should be revoked and rotated and the
  repository sharing history reviewed.

### What changed

- Moved all 156 PDFs into task-oriented
  [book folders](books/README.md): physics (90), mathematics (33), computation
  (17), programming (12), EdTech (2), and title-review research (2). The source
  filenames and PDF bytes are preserved; the two opaque filenames are
  quarantined for identification rather than guessed into a subject.
- Updated the project map, references index, architecture notes, and Work-Time
  Engine book links to use the new folders.
- Added the interactive motion lab to [index.html](index.html), styled it in
  [style.css](style.css), and connected its controls and drawing in
  [script.js](script.js). It is an original teaching interaction inspired by
  visual explanation, not copied 3Blue1Brown artwork or a replacement for the
  planned offline product.

### Verification and next action

- Verified: JavaScript syntax; 30 unique HTML IDs and all 25 local
  navigation/assets; Markdown links in 13 project documents; a DOM/canvas
  interaction smoke test for lesson setup, tabs, slider changes, animation,
  and pause; and byte-for-byte Git-blob comparison for all 156 moved PDFs.
  `git diff --check` passes. A visual browser test is still unavailable in this
  environment.
- Next: finish those checks, then continue extending the site with one focused
  visual explanation at a time; keep offline downloads, account services, and
  production forecasting as separate future work.

## 2026-11-08 (continued): instruction and book intake for the Work-Time Engine

Work item: review the latest saved instructions and route newly uploaded
reference books into the active Work-Time Engine matrix for continuity.

### Context reviewed

- Read [INSTRUCTIONS_2026-11-06.md](INSTRUCTIONS_2026-11-06.md),
  [STANDING_ORDERS.md](STANDING_ORDERS.md), the current
  [Work-Time Engine matrix](docs/qcos/WORKMATRIX.md), and the backend status.
  The saved instruction snapshot confirms the Work-Time Engine priority;
  slice 2 `cos_mat` is the next implementation slice.
- Identified new references for matrix/projection work, calculus and Fourier,
  physics, algorithms, computation structures, languages, and operating
  systems. The EdTech PDFs are now organized under [books/edtech/](books/edtech/).
- At that prior handoff, `pdftotext`, `pdfinfo`, `mutool`, and common Python
  PDF packages were absent, so the new Work-Time Engine references were left
  unread. In this continuation a local text-layer extraction was used for the
  Saem Global EdTech PDF only; the S2 mathematics sources remain unread. See
  the newest entry below for the targeted EdTech reading and website work.
- The newest saved instruction snapshot in the checkout is dated 2026-11-06.
  There is no newer `project_info` result here, so no new verbatim snapshot was
  fabricated.

### What changed and next action

- Added a filename-based, explicitly unread routing table to
  [WORKMATRIX.md](docs/qcos/WORKMATRIX.md), with a three-source reading batch
  for the next S2 work item and later-slice queues for the remaining books.
- Clarified in [references/README.md](references/README.md) that books are
  indexed by topic in the book library.
- Next: read the S2 batch (Strang CSE, `projection matrix.pdf`, and MIT 6.042)
  from extracted text before implementing `cos_mat`; log the specific sections,
  equations, code patterns, and failure modes then. No implementation or
  verification claims are made by this intake update.

## 2026-11-08 (continued): redesigning the public Saem Physics website

Work item: remake the public channel site, include useful teaching examples
for the requested programming languages, and prepare a truthful static
publication without claiming an unconnected C17 backend.

### Context and reading attempted

- Read the current [README.md](README.md), product architecture,
  [STANDING_ORDERS.md](STANDING_ORDERS.md) and its language-role guidance,
  backend [STATUS.md](saem-unified/docs/STATUS.md), [PROGRESS_c17.md](saem-unified/docs/PROGRESS_c17.md),
  and the recent project intuition entries.
- Checked the project book map. This checkout contains the two supplied EdTech
  PDFs in [books/edtech/](books/edtech/), but no extracted text or
  other matching books. `pdftotext`, `pdfinfo`, `mutool`, `pypdf`, `fitz`, and
  `pdfminer` are unavailable; therefore I did not attribute a new design rule
  or quotation to either PDF.
- Checked GitHub Pages configuration; the Pages API returned 404, so Pages
  was not configured at the start of this work item.

### Equation, invariants, and implementation choices

- Teaching example: for constant acceleration,
  `s = ut + (1/2)at^2`. With `u=4`, `a=2`, `t=3`, the exact integer result is
  `s=21`. The C17 tab performs the calculation through `CosRat`; the other
  language tabs use the same small integer inputs as illustrative sketches.
- Keep presentation and authority separate: HTML/CSS/JavaScript is the static
  public front end; the C17 QCOS core is a future bounded authority, not a
  currently hosted web service. GitHub Pages does not execute C17.
- Existing C17/C++ source, Python verification tools, and Rust archive retain
  their actual status. R, Julia, Fortran, and Lisp examples are not represented
  as integrated product runtimes.
- Failure modes to avoid: implying a formula sketch is a general overflow-safe
  solver, implying language examples are wired into the engine, or advertising
  a published backend when only static hosting is available.

### What changed

- Replaced the previous landing page with an editorial, responsive channel
  website that retains the existing YouTube, admission, Telegram, contact, and
  Webex live-class destinations.
- Added searchable/category-filtered lesson cards, keyboard-accessible
  language-example tabs with copy support, reduced-motion styling, and a clear
  statement of the static/backend boundary.
- Added a GitHub Pages Actions workflow that publishes only the three website
  files, leaving research PDFs, archives, and backend source out of the web
  artifact.
- Updated the root README, the language-role table and website status in the
  architecture, and this continuity log. Publication still needs to be
  enabled and verified after pushing.

### Verification and next action

- `node --check script.js` passes. A standard-library HTML parse found 21
  unique IDs and all 24 asset/navigation references resolve.
- Compiled and ran the C17 example against `saem-unified/core/c17/cos_rat.c`;
  it returned exact `21/1`. The C and C++ examples compiled with strict
  warnings, and the Python example evaluated to 21.
- A DOM interaction harness passed lesson search, category filtering, the
  empty state, language tab selection and arrow-key navigation, and menu open /
  outside-click close. No browser binary is installed, so this is not a
  visual-browser test.
- R, Julia, Fortran, Rust, and Lisp examples are included but those runtimes
  are not installed here and could not be executed.
- Local HTTP smoke test returned 200 for the homepage, stylesheet, and
  JavaScript; GitHub Pages was not configured at the start of this check.

Next: push the redesign, enable GitHub Pages for `main` at `/`, and verify its
published response before calling the site live.

## 2026-11-08 (continued): organizing the source tree and exact rational slice 1.5

Work item: arrange supplied materials without entangling the channel website
and continue the authoritative Work-Time Engine priority.

### Materials and structure

- Read the existing root [README.md](README.md), architecture and QCOS notes,
  [WORKMATRIX.md](docs/qcos/WORKMATRIX.md), the backend rational API,
  implementation, tests, and Fraction-oracle logic.
- Grouped QCOS design and the Work-Time Engine plan in [docs/qcos/](docs/qcos/).
  Grouped the supplied PDFs and source archives under [references/](references/),
  with indexes in each folder. The static website stays at the repository root;
  the restored C17/C++ source remains isolated in [saem-unified/](saem-unified/).
- The two supplied EdTech PDFs are not yet text-extracted. No applicable
  numerical-analysis book/source was present in the workspace; I derived and
  checked the rational reduction independently rather than claiming new book
  reading.

### Rational derivation and implementation

For reduced `a=n1/d1`, `b=n2/d2`, set `g=gcd(d1,d2)`,
`u=d1/g`, `v=d2/g`, and form `t=n1*v+n2*u` with signed two-limb arithmetic.
Then `h=gcd(|t|,g)`, and the reduced result is
`(t/h) / (u*(d2/h))`. This cancels only the denominator gcd shared with the
wide numerator before checking whether the final numerator and denominator fit
the symmetric 64-bit `CosRat` range.

Added explicit `{hi,lo}` operations and a bounded 128-bit-by-64-bit
long-division helper in `cos_rat.c`; there is no `__int128` or approximate
fallback. Tests now cover same-denominator sums whose unreduced numerator
exceeds 64 bits, operand order and sign, exact cancellation to an integer,
overflow with untouched output, and output aliasing an input.

### Verification and next step

- Independent Python `Fraction` oracle: 192,805 commands; 138,432 exact
  results, 53,032 legitimate refusals, and **0** exactly representable sums
  refused.
- `python3 tools/check_c17.py --quick`: all C17 O2 tests pass; 54/55 total
  checks because the linked RISC-V audit cannot run without `ld.lld`.
- `make test`: 35 JavaScript tests and 325 C++ differential cases pass; Python
  unittest discovery explicitly skips because this source snapshot has no
  Python test files.
- The full sanitizer suite was not rerun after this change; the prior baseline
  had a missing RISC-V linker and ThreadSanitizer startup failures in this
  container.

Next: slice 2 `cos_mat`—exact rational matrix operations, elimination, rank,
and the four subspaces. No RISC-V execution, student-facing forecasting claim,
or PDF-derived content is implied by these results.

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
- `references/archives/open-math-harness-main.zip`, its README and Cargo manifest: it is a Rust
  runtime for Open Math OS; the README says the complete runtime depends on that
  deployment environment, while a small example is locally self-contained.
  Lean checks proof candidates. This is not established as an offline physics
  solver for the product.
- `references/archives/xv6-riscv-fall19-xv6-riscv-fall19.zip`, its README: this is a 2019 RISC-V
  teaching OS and requires a RISC-V toolchain and QEMU to run. It is reference
  material, not the Saem production backend.
- [saem-unified-bundle.py.txt](references/source-snapshots/saem-unified-bundle.py.txt): its manifest lists
  216 SHA-256-checked files, including `core/c17`, the EdTech `core/src`, tests,
  and docs. The bundle was inspected, restored to [saem-unified/](saem-unified/),
  and all extracted files were independently compared with their manifest
  digests.

At this earlier point, PDF extraction was unavailable, so the EdTech books
had not been read. In the 2026-11-08 continuation at the top of this log, the
Global EdTech PDF's 686-page tree was extracted locally and pages 58-61 were
reviewed for the website. Its companion remains unread.

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
a last axis = time, relations and functions in the cells, and a calculus layer on top; so that anyone, and QCOS itself, can complete, upgrade and repair QCOS. Spec: `docs/qcos/WORKMATRIX.md`.
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
- Not changed, deliberately: PMP, Smepmp, the door and the OTP counter are designs only (`docs/qcos/RISCV_QCOS.md` sections 3 to 5); the lock's spin loop stays a plain test-and-set (a test-and-test-and-set refinement needs `cache & structure.pdf` read first).

### Next reading (planned)

- THE ASSEMBLY LAYER (the user's request of 2026-11-01 22:25 and the updated project instructions): `QCOS Compiler by SaemLabs.PDF` (their baseline: direct binary emitter, Zone 0 on register nodes, exporter and boot vector, CPU simulator),
  `HandP_RISCV(1..3)` (instruction formats and encodings), `riscv-unprivileged.pdf` (base ISA encodings), and the machine-readable `riscv-opcodes`. Also new in the project: `Digital_Design_and_Computer_Architecture_RISC (Sarah_Harris)`,
  `students digital guide - mit computer structure`, `Introduction to Assembly Language Programming`, `Programming_from_the_Ground_Up` (x86 assembly: useful for what an assembler is and does, not for RISC-V encodings).
- SIR parsing (strict JSON into a static node arena): `mit6_042js15_textbook.pdf` (recursive data types, structural induction), `Juhasz Modern C in embedded systems.pdf` (static memory patterns).
- Journal and receipts: `Principles of Computer System Design 2 mit.pdf` (all-or-nothing and before-or-after atomicity).
- Lock spin and false sharing: `cache & structure.pdf`, `HW Performance software system mit.pdf` (before changing the journal lock to test-and-test-and-set).
