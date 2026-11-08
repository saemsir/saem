# Saem Unified source tree

This directory is the separately restored 216-file Saem Unified snapshot.
The public channel website remains at the repository root.

Start with `docs/STATUS.md` and `docs/STANDING_ORDERS.md`. Source and tests in
this tree are independent of the root website:

```sh
make test
make c17
```

`make test` runs the JavaScript reference suite and C++ differential cases.
`make c17` runs the QCOS C17 build, algebra/oracle checks, RISC-V object audit,
and sanitizer tests. Results depend on local tool availability; see the root
`docs/SAEM_EDTECH_ARCHITECTURE.md` for the checks completed in this workspace.

Build outputs and Python bytecode are ignored by this directory's `.gitignore`.
