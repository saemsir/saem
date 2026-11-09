# Saem Physics

## Daily vision update — 2026-11-09

### Past

- Saem Physics has a live-in-repository static website and an original
  interactive motion explainer.
- The Saem Global EdTech reference book has been reviewed and its useful
  product principles recorded: offline-first learning, clear visual
  explanations, source provenance, and human review. See the
  [book review](books/edtech/README.md) and
  [daily intuition log](DAILY_INTUITION.md).
- QCOS has verified exact algebra and the rational matrix API in S2. The next
  planned Work-Time Engine slice is S3 relations, not a finished EdTech
  backend; see the [work matrix](docs/qcos/WORKMATRIX.md).

### Present

Today I reviewed the public 3Blue1Brown homepage for broad visual-learning
principles, then extended Saem Physics with its own illustrated mechanics
principles, then added an original student practice room and a clear map of
what works today versus what remains planned. Students can answer three
conceptual and numerical kinematics questions, read worked reasoning, and
resume local progress on the same device without an account. The interface
links the C17 exact arithmetic/matrix kernel and tests while plainly noting
that these are not web services. The channel feed still refreshes every six
hours in that earlier version. No 3Blue1Brown artwork was copied; the Saem
visual identity remains distinct.

### Present — 2026-11-08 (continued)

The public page now has an eight-question, 12-minute sample test with worked
review and topic-level results, plus an experimental topic-frequency explorer
that runs only on locally imported, cited, teacher-reviewed past-paper data.
It requires at least five papers across three earlier years and labels its
smoothed frequencies as exploratory—not an official or calibrated exam
prediction. Channel uploads show as dated update cards with click-to-play,
privacy-enhanced YouTube embeds; an on-demand player can show a public live
channel broadcast. The feed snapshot refreshes hourly and the open page checks
it every 15 minutes. YouTube's public Atom feed does not provide Community
posts or live-status detection. See the
[daily intuition log](DAILY_INTUITION.md) for model details and test evidence.

The requested NEET/JEE exam-product scope is staged in the
[Exam Time Matrix](docs/EXAM_TIME_MATRIX.md). The three paper folders contain
papers the owner says they have personally verified; machine-readable
metadata, question extraction, and item-level solution mapping are in progress.
The versioned question/paper/rule contract and a checksummed inventory of 202
source files are now implemented. The intended product is a dynamic student
website backed by a versioned C17 service; its first loopback-only kernel/API
slice is implemented in [dynamic-site/](dynamic-site/), while the current
GitHub-Pages website remains static.

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
5. Keep the YouTube feed workflow green and review the published latest-video
   cards when channel uploads or the feed format changes. Build student
   practice from teacher-reviewed content with source and solution provenance.

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
It includes a searchable lesson library, hourly refreshed channel uploads with
click-to-play embeds and an on-demand channel live-stream player, an interactive
motion explainer, guided practice, and a local timed sample test with worked
answer review. Its experimental topic-frequency explorer accepts cited,
teacher-reviewed paper history from a local
JSON file; it does not claim to make a calibrated or official exam prediction.
The map separates tested repository features from planned product
capabilities. The site keeps the channel, playlist, admission, Telegram, and
live-class links.

## Run locally

From the repository root:

```sh
python3 -m http.server 8000
```

Open `http://localhost:8000`. Stop the server with Ctrl+C.

## Publish with GitHub Pages

The [Pages workflow](.github/workflows/deploy-pages.yml) deploys the website
assets and generated channel-feed snapshot when changes are pushed to `main`
or on its hourly refresh. It intentionally excludes research books, archives,
and backend source from the Pages artifact. In GitHub, open **Settings → Pages**
and select **GitHub Actions** as the build and deployment source. No build
runtime is required to serve the site.

The current public site is static: GitHub Pages does not execute the C17
backend. The future exam product is intended to be a dynamic student website
calling a separately deployed C17-backed service; that service and its
backend-capable hosting are not implemented yet. The language examples are
teaching sketches, not integrations. See the
[staged architecture](docs/SAEM_EDTECH_ARCHITECTURE.md).

## Project map

- [index.html](index.html), [style.css](style.css), [script.js](script.js),
  [student-studio.js](student-studio.js): public website, student practice,
  and browser interactions.
- [docs/qcos/](docs/qcos/): QCOS design and Work-Time Engine plan.
- [Exam Time Matrix](docs/EXAM_TIME_MATRIX.md): staged paper, mock-test,
  forecast, kernel, and dynamic student-website gates.
- [Dynamic-site prototype](dynamic-site/README.md): local website and API
  calling the existing C17 exact-rational kernel.
- [Work-Time Engine guide](docs/qcos/WORK_TIME_ENGINE_GUIDE.md): fixed slice
  order, verification gates, and session handoff procedure.
- [books/](books/): supplied physics, mathematics, computation, programming,
  and EdTech reference PDFs, grouped by task.
- [references/](references/): source archives and source snapshot, kept
  separate from deployable site assets.
- [saem-unified/](saem-unified/): restored C17/C++ source, tests, and tools.
