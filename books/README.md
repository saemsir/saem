# Saem reference library

The supplied PDFs are grouped by their primary use. Original filenames and
PDF contents are preserved; only their folders changed. This is a working
library, not a claim that every source has been read or independently checked.

| Folder | PDFs | Best starting points |
|---|---:|---|
| [Physics](physics/) | 90 | Mechanics, electromagnetism, quantum physics, relativity, and cosmology |
| [Mathematics](mathematics/) | 33 | Algebra, geometry, calculus, probability, number theory, and linear algebra |
| [Computation](computation/) | 17 | Algorithms, computer architecture, information theory, and operating systems |
| [Programming](programming/) | 12 | C, C++, Python, Rust, Julia, and Fortran |
| [EdTech](edtech/) | 2 | Saem product vision, teaching experience, platform design, and implementation proposals |
| [Research - review titles](research/) | 2 | PDFs with opaque filenames; identify their contents before relying on them |

## Routes by current work

- **Visual physics website:** [Saem Global EdTech](edtech/Saem%20Global%20EdTech.pdf),
  especially its visual-learning and mobile-performance discussion (PDF pages
  58-61). The current static site implements a small, original Canvas motion
  explainer; it does not copy another channel's assets or claim to be the
  proposed full EdTech platform.
- **Work-Time Engine, slice 2:** Gilbert Strang's
  [Computational Science and Engineering](mathematics/Gilbert%20Strang%20MIT%20Computational%20Science%20and%20Engineering%20%281%29.pdf),
  [projection matrix](mathematics/projection%20matrix.pdf), and
  [MIT 6.042](mathematics/mit6_042js15_textbook.pdf).
- **RISC-V, operating systems, and compiler work:** start with the xv6 and
  computation-structures references in [Computation](computation/), then use
  the matching systems notes in `saem-unified/docs/`.
- **Language/runtime work:** select the language-specific source in
  [Programming](programming/); its availability does not imply integration in
  the website or backend.

The [EdTech folder index](edtech/README.md) records the full-document review
of Saem Global EdTech and an important credential-handling warning. For all
other reading, record the specific sections, equations or invariants, code
patterns, and failure modes in [DAILY_INTUITION.md](../DAILY_INTUITION.md).
