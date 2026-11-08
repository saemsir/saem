# Saem EdTech books

- [Saem Global EdTech.pdf](Saem%20Global%20EdTech.pdf) — 686 pages in the
  authoritative PDF page tree. Full-document text review completed: all 686
  pages yielded text (91,466 words; no page extraction errors). This is a
  compiled set of evolving architecture proposals, not one consistent,
  implementation-ready specification.
- [Saem EdTech Research and Codespaces Build Book.pdf](Saem_EdTech_Research_and_Codespaces_Build_Book.docx.PDF)
  — supplied companion book; not yet reviewed in this task.

## Full-book review and implementation boundary

The PDF covers an AI-SLAVE neuro-symbolic architecture and immutable-core /
adaptive-shell split (pp. 1-22, 180-611), Saem Physics business and platform
proposals (pp. 23-150), learning, assessment, forecasting, and visual pedagogy
(pp. 151-180), and model/mobile implementation proposals (pp. 612-686). Its
outline and text layer were reviewed across all pages; code samples and
third-party or market claims were not independently validated.

### Useful principles retained

- Keep the student experience calm and legible; use visual explanations to
  expose a mathematical idea rather than add interface noise (pp. 58-62,
  151-177).
- Treat offline access, low-end-device performance, and static fallbacks as
  explicit product constraints; the proposed 45 FPS fallback is a target in
  the book, not a measured result in this site (pp. 58-59).
- Put deterministic checking alongside generated math, preserve source and
  question provenance, and keep human review in important learning/data flows
  (pp. 58, 151-162, 360-362). These are goals, not a guarantee of
  "zero hallucination."
- Separate immutable rules from adaptable content and measure resource use
  before making device-performance claims (pp. 1-4, 201-205, 265-269).
- An A* search score is written as `f(n) = g(n) + h(n)` (p. 2); the book does
  not establish the heuristic conditions needed to use it as a WTE guarantee.

### Boundaries and cautions

- The book proposes several incompatible stacks and deployment paths,
  including static HTML, Next.js/React Native, AWS services, Python/FastAPI,
  Python/Kivy, and a pure-C core. Do not combine these proposals or treat any
  one as the current product. The staged architecture and tested repository
  code govern implementation.
- Symbolic computation can check specified equations or arithmetic; it does
  not by itself prove a generated explanation, source citation, unit model, or
  forecast is correct.
- The book includes automatic AI code repair/commit examples, client/server
  security examples, and credential-like sample values. Treat all samples as
  untrusted and do not copy credentials or enable unattended changes to the
  project.
- The current public site uses a small, user-controlled Canvas visualization
  and retains its own visual identity. It is an original teaching interaction,
  not copied 3Blue1Brown artwork and not the proposed offline learning app.

## Credential safety

The original Global EdTech PDF contains a credential-like value in an example
configuration. Do not copy or deploy it. If it is or was a real credential,
revoke and rotate it, and review the repository's sharing history. The supplied
PDF is preserved unchanged; this warning deliberately does not reproduce the
value.
