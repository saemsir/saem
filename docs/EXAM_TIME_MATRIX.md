# SAEM Exam Time Matrix

Status: staged product plan, updated 2026-11-09. This is the exam-product
time matrix; it does not change the QCOS Work-Time Engine slice order in
[docs/qcos/WORKMATRIX.md](qcos/WORKMATRIX.md). **Verified** means code or a
test has demonstrated the behavior. **Planned** means it has not.

## 1. Product boundary

Build a calm, student-first NEET, JEE Main, and JEE Advanced learning workflow:
transparent topic outlooks, timed practice, exam-rule scoring, worked
solutions, and a study path based on results. Predictions are evidence-based
estimates, never promises about future questions.

The intended destination is a dynamic student website backed by a versioned
service that hosts the C17 kernel for its supported, bounded operations. The
website frontend presents the learning experience; a server-side adapter
validates requests and invokes the C17 service through a narrow, tested
interface. The C17 kernel is not itself a website or HTTP server, and the
current GitHub Pages site remains static until a separate backend-capable
deployment is built and selected. Keep student data private by default and
make storage, accounts, and sharing explicit design decisions.

The supplied books are product context, not executable specifications. The
recorded EdTech principles are clear visual reasoning, offline/low-device
access, source provenance, deterministic checking, and human review. Keep
unvalidated forecasts and generated solutions visibly distinct from verified
answers; never claim "zero hallucination."

## 2. Exam work-time matrix

| Slice | Work | Exit evidence | Status |
|---|---|---|---|
| E0 Corpus inventory | Map each source file to exam, session/year, subject, language, source, and permitted use. Preserve originals; identify missing or invalid files; do not infer a full exam from a single question file. | A checked inventory has stable source IDs, no unresolved duplicates, and explicit rights/provenance status. | **In progress:** generated SHA-256 inventory covers 202 files; matching existing files retain the owner's paper-level attestation, while new files are marked `not_reviewed`; filename-derived values are hints only. The two local placeholders were deleted at the owner's request. Opaque filenames, permissions, and source citations still need indexing. The local dynamic site watches the three source folders and refreshes the manifest when PDFs are added, changed, or removed. |
| E1 Content contract | Define versioned records for exam rules, papers, questions, topics, solutions, language, source references, and review state. Keep extracted source text separate from normalized/search text. | Schema and boundary tests cover malformed, incomplete, duplicate, unsupported, and valid records. Every imported item retains provenance. | **DONE, verified:** JSON Schema 2020-12 plus cross-reference, file-hash, permission, and mock-eligibility validator; 9 contract tests pass. No exam questions have been extracted into a dataset yet. |
| E2 Import and review | Create a local import workflow. Treat PDF extraction as a proposal; the owner or delegated reviewer confirms question boundaries, diagrams, options, marks, topic labels, language, answer key, and solution. Preserve the owner's existing paper-level verification separately from item-level extraction review. | A reviewer can approve, reject, or correct each item; no item with unresolved extraction, answer, or solution review enters a student mock or forecast set. | Planned |
| E3 Student mock tests | Select exam, subject, year/session, language, duration, question types, and the matching rule profile. Support each format only after its interaction and scoring semantics are implemented. Give navigable questions, timer, progress, submit confirmation, score, and worked review. | Exact rule-profile fixtures and end-to-end tests cover scoring, unanswered items, timing, navigation, and every supported question type. Unsupported formats are refused, not approximated. | **Partial, verified:** one original eight-question physics sample has a timer, navigation, scoring, topic results, and worked review. It is not a NEET/JEE mock. |
| E4 Topic outlook | Estimate topic coverage from eligible prior papers only. Show source years, paper count, language, uncertainty/coverage, and a historical baseline. Hold out each target year during back-testing. | Reproducible past-only evaluation reports calibration/coverage and Brier score against a baseline; target-year and later data never leak into a forecast. | **Exploratory, verified:** imported reviewed topic-history JSON uses `p = (k + 1)/(n + 2)` and excludes the target year. No curated dataset or back-test exists. |
| E5 Question prediction | Add question-pattern or question-level ranking only after sufficient reviewed data and past-only validation. Distinguish similarity/retrieval from probability; attach evidence to every result. | Held-out evidence supports the stated claim; outputs include uncertainty and provenance, and suppress unsupported predictions. | Planned; no question-level prediction is claimed. |
| E6 Learning loop | Turn a completed test into topic-level feedback, a revision path, and optional new practice. Keep the student in control; keep identity and answers local unless sharing is explicit. | Feedback is reproducible from the attempt and reviewed topic map; local privacy behavior and accessibility are tested. | Planned beyond current local progress and sample-test review. |
| E7 Engine and integration | Keep exam semantics in a versioned domain contract. Use C++17 domain code where verified; use C17/QCOS only for exact, bounded operations behind an explicit service interface. Keep forecasting statistics, question content, and kernel arithmetic separate. | Contract, differential, refusal, resource-bound, and end-to-end tests pass before integration. The adapter rejects malformed/out-of-range input and surfaces kernel refusal; it never invents a successful fallback. | **In progress:** D0's local C17 bridge/API is verified; exam content, rules, and scoring are not yet integrated. |
| E8 Dynamic student website | Evolve the current static frontend into a dynamic website that calls authenticated or otherwise explicitly bounded backend endpoints hosting the C17 service. Select hosting/runtime only after resource, privacy, threat-model, and operating-cost requirements are known. | A student can use a reviewed mock through the deployed website; API contracts, authorization/rate/resource limits, privacy, recovery, deployment, and actual-device behavior are tested. Only approved content is published. | **In progress:** local dynamic-site prototype works on loopback; current GitHub Pages site remains static and no public C17 service is deployed. |

### Active implementation queue

| Step | Deliverable | Exit evidence | Status |
|---|---|---|---|
| D0 Kernel-to-browser vertical slice | Local dynamic site, versioned HTTP API adapter, and a narrow exact-rational operation calling the existing C17 `cos_rat` implementation. Bind development server to loopback; bound requests; publish no paper files. | API tests cover exact answers, invalid inputs, C17 refusals, response limits, and route behavior; browser UI calls the API; no C source is copied or reimplemented in JavaScript. | **DONE, verified locally:** exact-rational API, safe loopback serving, live paper-folder inventory watcher/API, and focused integration tests pass. Not a public deployment. |
| D1 Paper/question contract | Versioned paper, question, rule, language, topic, answer, solution, and source records; import metadata while preserving PDFs. | Schema validates against the owner's verified source set; per-question extraction/review status is explicit. | **DONE:** contract is implemented; content mapping is a separate next task. |
| D1a Source mapping | Resolve filename hints and missing-file anomalies into owner-confirmed exam year/session/subject/language/source metadata; keep redistribution permission separate. | Owner-confirmed mappings cite each source file; bad/placeholder files are replaced or explicitly excluded; no metadata is inferred from opaque IDs. | **NEXT** |
| D2 Exam mock engine | Implement a single selected exam/subject/rule profile from reviewed items, then add profiles with focused tests. | Exact marking, timing, navigation, question-type, language, and worked-review fixtures pass. | Pending |
| D3 Forecast and evaluation | Build topic coverage and question retrieval/ranking only from the verified, mapped history; run past-only holdouts. | Coverage, uncertainty, calibration, and baseline comparisons are reproducible; future/held-out papers are excluded. | Pending |
| D4 Public dynamic deployment | Deploy frontend and API/kernel adapter to compatible hosting after operational and privacy requirements are settled. | Production build, end-to-end tests, monitoring/limits, privacy/security review, and target-device checks pass. | **Private preview scaffold prepared:** root Dockerfile builds the C17 adapter, excludes papers from the image, and keeps Cloud Run IAM authentication required. Deployment remains pending an authorized Cloud Run project/region and live preview verification; this is not a public student service. |

### Current source-folder inventory (file count only)

The repository currently has 9 PDFs in [neet_exam/](../neet_exam/), 156 in
[jee_exams/](../jee_exams/), and 37 in [jeeadv_exams/](../jeeadv_exams/).
These are raw-file counts, **not** counts of distinct papers or verified exam
sessions. The owner states that all supplied papers have been personally
verified; that owner attestation is the content-verification status for this
corpus. Many JEE Main filenames are opaque `que_...` names, so year, session,
subject, language, and source still need indexing. The generated inventory
records SHA-256 and filename-only hints. The two 2-byte CRLF placeholder
files were deleted at the owner's request; the refreshed inventory contains
202 files and all pass the first-1024-byte PDF-header check. The inventory
also recognizes the JEE Advanced 2017 paper header after its leading CRLF.
These are machine file observations, not a challenge to the owner's
paper-content verification. No question text was extracted for this matrix.
New PDFs placed in these three folders are discovered on the next inventory
refresh, including automatically when the local dynamic site starts.

Do not copy raw PDFs or their question text into the public Pages artifact.
Before redistributing paper content in a public question bank, record the
source and verify applicable permission. The owner's paper verification is
respected; extracted questions, answer keys, labels, and solutions still need
traceable item-level mapping and review before the engine can safely use them.

## 3. Exam data matrix

Each record should preserve these axes and relations:

| Axis | Required meaning |
|---|---|
| Exam | NEET, JEE Main, or JEE Advanced; do not merge unlike rule profiles. |
| Paper | Stable ID, year, session/shift, paper/part, and source reference. |
| Subject and language | Subject plus original language/script; never silently translate or normalize away the original. |
| Question | Stable ID, source location, question type, prompt/assets, options or response format, marks, and topic labels. |
| Solution | Answer key, concise steps, units/assumptions, author/source, review state, and version. |
| Rules | Year/session-specific duration, section behavior, attempt limits, positive/negative marks, and rounding/scoring policy. |
| Evidence | Eligible past-paper IDs, topic definitions, method/version, held-out years, and measured validation. |
| Time | Paper year is evidence time; learner attempt time is private activity time. Neither is the QCOS append-only kernel time axis. |

Core relations are `source -> paper -> question -> topic`, `question ->
solution`, `paper -> rule profile`, and `forecast -> eligible prior papers`.
Every forecast for target year `T` must satisfy `paper.year < T`. A paper's
review flag is not proof by itself: review decisions and changes need a
traceable record in the local content workflow.

The simplest forecast baseline is a declared smoothed, weighted topic rate:

`p(topic) = (sum_i w_i * y_i + alpha) / (sum_i w_i + 2 * alpha)`

Here `y_i` indicates whether eligible paper `i` includes the topic, `w_i` is a
declared paper weight, and `alpha > 0` is a declared smoothing value. Select
weights and smoothing using past-only validation. Report a proper score such
as `Brier = mean((p_i - y_i)^2)` and compare against a historical-frequency
baseline. A topic frequency is not a probability that a particular question
will appear.

## 4. Dynamic website and kernel relationship

The exam matrix is content/product data, not an instruction to put PDFs,
question text, or a predictive model inside the QCOS kernel. The intended
dynamic website can call a future C17-backed service, but the service must be
a separate adapter around the kernel—not HTTP or account logic added to the
bounded core. Keep the public frontend, server/API adapter, C17 kernel, and
question/content store as distinct trust boundaries.

The currently verified kernel capabilities include exact rational arithmetic
and a rational-matrix API; they do not include exam-specific scoring, topic
forecasting, a question bank, a web API, or automatic content review. A
versioned API must define supported operations, input/output bounds,
authentication/authorization policy, timeouts, rate/resource limits, error
mapping, and audit/privacy behavior before the frontend relies on it.

Possible later use of the kernel is limited to a separately specified,
bounded calculation whose inputs and exact/refusal behavior have tests.
Scoring rules and solution checks must first be defined in the exam-domain
contract; any kernel adapter must preserve those rules exactly. Do not skip
the ordered S3-S7 Work-Time Engine gates or claim that S2 already powers this
product. Hosting C code does not make an unimplemented capability available:
the kernel and adapter must be built, deployed, monitored, and tested as a
service.

## 5. Next work order

1. Complete D1a by resolving filename hints, source citations, and the two
   local placeholder files; keep the owner attestation and technical checks as
   separate fields.
2. Use the completed E1 contract to map one owner-verified paper and its
   exact rule profile before building a board-specific mock.
3. Implement E2 locally, then E3 for explicitly supported question types and
   rule profiles; keep the existing generic sample clearly labelled.
4. Build a curated topic history and past-only E4 back-test before describing
   a forecast as predictive. Keep E5 question ranking blocked until its own
   held-out evidence exists.
5. Continue the kernel only in its saved order. Integrate the exam domain and
   kernel through tests, not shared assumptions.
6. Deploy the IAM-protected Cloud Run developer preview only after an
   authorized project and region are supplied; keep exam PDFs out of the
   image. Design any public student API only after the content contract, C17
   adapter boundary, privacy policy, and hosting/runtime requirements are
   explicit; keep the existing static site available during that transition.

The [daily intuition log](../DAILY_INTUITION.md) records the context, sources,
and decisions used for each completed work item.
