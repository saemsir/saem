# Saem Physics

## Daily vision update — 2026-11-08

### Past

- Saem Physics has a live-in-repository static website and an original
  interactive motion explainer.
- The Saem Global EdTech reference book has been reviewed and its useful
  product principles recorded: offline-first learning, clear visual
  explanations, source provenance, and human review. See the
  [book review](books/edtech/README.md) and
  [daily intuition log](DAILY_INTUITION.md).
- QCOS has verified exact algebra and rational foundations. The next planned
  Work-Time Engine implementation is matrix support (S2), not a finished
  EdTech backend; see the [work matrix](docs/qcos/WORKMATRIX.md).

### Present

Today I completed the rational S2 `cos_mat` API: bounded exact RREF/rank, four
fundamental-subspace bases, consistent/inconsistent solving, inverse, and
orthogonal projection (including rank-deficient inputs). C tests cover exact
results, projector symmetry/idempotence, subspace invariants, refusals, and
untouched outputs (183 checks); the independent Fraction oracle verifies 503
matrices, 250 systems, 100 inverses, and 150 projections. The integrated quick
suite still has one unrelated environment block: the RISC-V link audit needs
unavailable `ld.lld`. Frontend/backend integration remains future work.

### Future

Build the student and teacher experience, the offline EdTech workflow, and
QCOS's exact-math capabilities in parallel where their interfaces do not
conflict. Keep each lane small and testable, preserve the current boundaries,
and integrate only through documented, verified interfaces. The
[staged architecture](docs/SAEM_EDTECH_ARCHITECTURE.md) and
[three-lane work order](docs/qcos/WORKMATRIX.md#21-cross-domain-parallel-work-order)
describe that path.

## What you can do to complete the SAEM Global EdTech vision

### Inside GitHub

1. Keep the three lanes visible in the
   [work matrix](docs/qcos/WORKMATRIX.md); take one small, testable next step
   in each lane when dependencies allow, and record what is planned versus
   verified.
2. Continue the kernel in its required order: advance to S3 relations only
   after this verified rational S2 checkpoint; keep later matrix-algebra
   extensions explicit rather than implying they are already implemented.
3. Keep the product boundaries explicit: the static site is not the offline
   app, the C++17 EdTech engine is not exposed as a website API, and the C17
   QCOS kernel is not the customer-facing application.
4. Add code, tests, and source/provenance notes together. Keep forecasts
   transparent, avoid promises about exam questions, and require human review
   for imported or generated learning content.

### Outside GitHub

1. Choose the first learner group, exam boards, launch region, and languages;
   decide which learning workflow should be useful first.
2. Gather question papers and learning materials you have permission to use.
   Record their board, year, language, and source, and arrange teacher review
   for extracted questions and solutions.
3. Speak with students and teachers about device limits, offline use,
   accessibility, and the mock-test experience; use their feedback to
   prioritize the next small feature.
4. Set a sustainable routine: make one manageable daily contribution, then
   review progress and choose the next step rather than trying to build the
   whole vision at once.

The public front door for Saem Physics is a responsive, static website for
students exploring lessons, exam solutions, and physics through computation.
It includes an original, interactive motion explainer, keeps the channel,
playlist, admission, Telegram, and live-class links, and offers a searchable
lesson library with short code examples in the requested languages.

## Run locally

From the repository root:

```sh
python3 -m http.server 8000
```

Open `http://localhost:8000`. Stop the server with Ctrl+C.

## Publish with GitHub Pages

The [Pages workflow](.github/workflows/deploy-pages.yml) deploys the three
website files (`index.html`, `style.css`, and `script.js`) when changes are
pushed to `main`. It intentionally excludes the research books, archives, and
backend source from the Pages artifact. In GitHub, open **Settings → Pages**
and select **GitHub Actions** as the build and deployment source. No build
runtime is required to serve the site.

The public site is static: GitHub Pages does not execute the C17 backend. The
language examples are teaching sketches, not integrations. The intended
offline EdTech application and the current state of each language are described
in [the staged architecture](docs/SAEM_EDTECH_ARCHITECTURE.md).

## Project map

- [index.html](index.html), [style.css](style.css), [script.js](script.js):
  public website and browser interactions.
- [docs/qcos/](docs/qcos/): QCOS design and Work-Time Engine plan.
- [Work-Time Engine guide](docs/qcos/WORK_TIME_ENGINE_GUIDE.md): fixed slice
  order, verification gates, and session handoff procedure.
- [books/](books/): supplied physics, mathematics, computation, programming,
  and EdTech reference PDFs, grouped by task.
- [references/](references/): source archives and source snapshot, kept
  separate from deployable site assets.
- [saem-unified/](saem-unified/): restored C17/C++ source, tests, and tools.
