# Saem Global EdTech: staged product and systems architecture

Status: architecture proposal for this workspace, 2026-11-08. It separates
verified material from planned work; it does not claim that a production
backend or RISC-V deployment already exists.

## Product direction

Start with the Saem Physics YouTube channel as the public front door. Grow it
into an offline-first learning product for teachers and students: lessons and
notes, exam-paper practice, transparent topic forecasts, timed mock tests,
worked solutions, and a study plan based on results. India exam boards and
JEE/NEET are the first content context; add other regions only with their own
reviewed curricula and paper data.

The forecast must say **chances, never promises**. Store the source and language
of each question and solution. Keep student work private by default, allow
teacher/classroom workflows only with explicit sharing, and do not make a
forecast or an automated model a substitute for a teacher.

## System boundaries

```text
Public channel site (HTML/CSS/JavaScript; static hosting)
                  |
                  v
Local/offline learning app (Python UI/server and import/export)
                  |
             versioned JSON
                  v
EdTech domain engine (C++17 modules from the Saem source snapshot)
        |                         |
        v                         v
QCOS C17 gate and exact       Open Math Harness (Rust)
math services                 isolated research/verification tool
        |
        v
Later: constrained RISC-V target and local/edge deployment
```

These are separate trust and deployment boundaries. The root static website can
be published without a server; it must not pretend to run the exam forecaster.
The RISC-V kernel is not a web server. Open Math Harness is not a drop-in
arithmetic library or a dependency that may silently decide student results.
Initially the learning app should work on one computer with local files and a
local database; classroom access on a local network is a later, opt-in mode.

### Programming-language roles

Languages meet at documented data formats and tested interfaces; they are not
all compiled into the browser or placed in one runtime.

| Language | Bounded role |
|---|---|
| HTML, CSS, JavaScript | Public website and browser presentation. Keep this first channel page static and lightweight. |
| Python | Local/offline application shell, import/export, UI orchestration, and test automation. It must not be trusted to change QCOS-protected state. |
| C++17 | Existing EdTech domain/solver and forecast modules in the archived Saem source bundle; isolate behind the JSON-lines/API boundary and preserve differential tests. |
| C17 | QCOS/saem kernel, gates, bounded state, and exact rational/algebra primitives. Treat the kernel as a narrow authority/verifier, not the whole customer-facing app. |
| C | The website's cross-language section includes a small integer-arithmetic example for comparison; production backend code remains C17. |
| Rust | Open Math Harness itself and, if useful, standalone local developer/administration tools. Use a process/JSON boundary first; do not add unverified FFI ownership or runtime coupling. |
| R | Offline statistics exploration, visualizations, and forecast analysis. Promote a result only after reproducible tests and porting it to a supported runtime. |
| Fortran | Offline numerical prototypes and bounded numerical routines; specify integer/rational semantics before any routine becomes part of exam scoring. |
| Julia | Offline mathematical experimentation and benchmarks only. No Julia runtime or garbage collector in the production deterministic backend. |
| Lisp | Optional symbolic-language learning and expression experiments; no Lisp runtime is currently integrated into the product. |

This uses the requested languages where their strengths fit, while keeping the
actual browser interface coherent. It avoids making students download multiple
language runtimes to view a lesson.

### Public website status

The root website is an HTML/CSS/JavaScript static site. It presents existing
YouTube lessons and links, filters the curated lesson list in the browser, and
shows small kinematics examples for C17, C, C++17, Python, Rust, R, Julia,
Fortran, and Lisp. Those examples are educational sketches, not a polyglot
runtime or calls into the EdTech engine. Its C17 example uses the existing
`CosRat` arithmetic API, while the other snippets use small integer inputs to
illustrate the same equation.

GitHub Pages can serve that static site, but cannot execute a C17 service. The
QCOS C17 source and C++ EdTech engine remain in the separate
[`saem-unified/`](../saem-unified/) tree and are not yet exposed through a web
API. A future offline app should connect to bounded C17 capabilities through a
versioned, tested interface; hosting and deployment of that service are a
separate stage.

## Learning and computation flow

1. Import a paper and preserve its original file, board, year, language/script,
   and provenance. Extraction is a proposal until a teacher confirms question
   boundaries, marks, and topic labels.
2. Maintain a versioned question/topic taxonomy and a review trail. Keep source
   text distinct from normalized search text so Devanagari and other scripts
   are not lost or silently rewritten.
3. Generate a forecast from eligible prior papers. A simple, auditable topic
   baseline to compare against is a smoothed weighted occurrence rate:

   `p(topic) = (sum_i w_i * y_i + alpha) / (sum_i w_i + 2 * alpha)`

   Here `y_i` is whether the topic appeared in paper `i`, `w_i` is a declared
   recency/eligibility weight, and `alpha > 0` is a declared smoothing value.
   The formula is a proposed baseline, not a result or a model already in this
   repository. Choose weights and smoothing using past-only validation; never
   train on the paper being evaluated.
4. Back-test by holding out one year at a time and fitting only on earlier
   papers. Report coverage, calibration, and a proper score such as the Brier
   score `mean((p_i - y_i)^2)` alongside a historical-frequency baseline.
   Display data coverage and uncertainty; suppress forecasts when the evidence
   is insufficient. A ranked topic is not a guarantee that a question will
   appear.
5. Assemble a mock test only from reviewed questions. Score deterministically
   under the selected exam's marking rules; attach solution steps, source
   provenance, and a version to every answer key.
6. Summarize weak topics and suggest a study plan. Keep the student in control
   and make the teacher's review and correction path explicit.

The exam rule set, marks, negative marking, timers, and output language must be
data/configuration with tests, not hidden assumptions in UI code.

## What the supplied materials support

### Present in this repository

- [index.html](../index.html), [style.css](../style.css), and
  [script.js](../script.js): an existing one-page Saem Physics channel site
  with video, playlist, topic, admission, Telegram, and live-class links.
- [The organized book library](../books/README.md) includes the
  [Saem Global EdTech book](../books/edtech/Saem%20Global%20EdTech.pdf), whose
  PDF page tree contains 686 pages. Its visual-learning and performance notes
  were reviewed for this website update (pages 58-61). The companion
  [Research and Codespaces Build Book](../books/edtech/Saem_EdTech_Research_and_Codespaces_Build_Book.docx.PDF)
  remains unread; no detailed claims are attributed to it.
- [INSTRUCTIONS_2026-11-06.md](../INSTRUCTIONS_2026-11-06.md),
  [STANDING_ORDERS.md](../STANDING_ORDERS.md), and
  [saem-unified-status.md](../saem-unified-status.md): persistent product,
  technology, and continuity notes. Older statements in them mention different
  prototype constraints; resolve conflicts against the latest user direction
  and this staged architecture rather than treating every historical note as
  current code.
- [saem-unified-bundle.py.txt](../references/source-snapshots/saem-unified-bundle.py.txt): a
  SHA-256-checked snapshot containing 216 files, including `core/c17`,
  `core/src`, tests, and `docs`. It has been restored to the separate
  [saem-unified/](../saem-unified/) directory; all 216 extracted files match
  their embedded SHA-256 digests. Keep its build outputs separate from the
  root channel site.
- [open-math-harness-main.zip](../references/archives/open-math-harness-main.zip): Rust source for
  Open Math Harness. Its README says the full runtime depends on the Open Math
  OS deployment environment; only a small example is self-contained locally.
  It uses Lean to check candidate proofs, not as a general physics solver.
  Evaluate its license, dependencies, security boundary, and offline behavior
  before adopting code or running student workloads.
- [xv6-riscv-fall19-xv6-riscv-fall19.zip](../references/archives/xv6-riscv-fall19-xv6-riscv-fall19.zip):
  the 2019 MIT teaching xv6/RISC-V source archive. It is a study reference, not
  Saem's production kernel or a ready-to-deploy application backend.
- [README.md](../README.md): corrected instructions for the current root-level
  static website.

The supplied books and archives are kept unmodified under `references/`;
add reviewed notes and licensed integrations beside them, not into their
contents.

## Delivery stages

| Stage | Deliverable | Exit condition |
|---|---|---|
| 0 — Channel front door | Publish the current static channel page and verify its links, metadata, mobile layout, and keyboard navigation. | A student can reach lessons and the existing learning/contact resources; no backend claims are made. |
| 1 — Recover the EdTech baseline | Restore the checked Saem source bundle into a clearly named subdirectory, inspect its exact build/test commands, and reconcile its recorded status with current source/tests. | Clean, repeatable build and tests; no overwriting the channel site. |
| 2 — Local learning shell | Connect a minimal Python UI to the existing C++ engine through a documented, bounded JSON interface; keep local content and progress on device. | Import, reviewed question display, and one mock-test flow work offline with integration tests. |
| 3 — Auditable forecasting | Add paper normalization, reviewed labels, past-only back-tests, calibration reporting, and explicit low-evidence behavior. | Reproducible evaluation beats or explains its baseline; every output has provenance and uncertainty. |
| 4 — QCOS and mathematics | Use QCOS gates/exact math where the restored implementation actually supports a calculation. Evaluate Open Math Harness separately for proof-verification work; keep it outside the scoring authority until independently validated. | Cross-language contract tests, exact-answer or explicit-refusal behavior, and no unsupported proof/forecast claims. |
| 5 — RISC-V and scale | Port or deploy bounded components to a specified RISC-V board/simulator only after the software contract is stable. | Target-specific builds, simulator or hardware tests, measured resource limits, and a recovery path. |

### Stage 1 verification in this workspace

- **Passed:** restored snapshot contains 216/216 files with matching hashes;
  JavaScript reference tests pass 35/35; C++ build and differential suite pass
  325/325 cases.
- **Passed:** `make test` now completes the JavaScript and C++ suites and
  explicitly reports that Python unittest discovery is skipped because the
  snapshot has no `test_*.py` files. No Python tests are claimed.
- **Partially blocked:** the QCOS check passes its strict C builds, header
  checks, zero-heap/global-state audits, RV32/RV64 encoding and decode
  comparisons, exact-rational oracle (192,805 commands), and algebra checks.
  Full completion is blocked by unavailable `ld.lld` for the RISC-V linked
  image audit and ThreadSanitizer's `unexpected memory mapping` in the
  container. Record these as environment limitations, not as passing checks
  or proven source defects.

## Immediate continuation

The smallest requested product step is the channel landing page, not a
simulated learning backend. The page already exists; the current change makes
it shareable and fixes its publishing instructions. The 216-file snapshot is
now separately restored and its C++/JavaScript baseline has been checked.
Resolve the test-environment blockers and missing Python tests before calling
the recovered baseline fully green. The next core-engine change in the saved
priority order is rational slice 1.5; read and log its matching math and C
sources before editing. Review and index the two EdTech PDFs with an available
extractor before claiming their detailed ideas have been incorporated.
