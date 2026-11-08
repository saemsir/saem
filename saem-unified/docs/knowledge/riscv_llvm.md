# What RISC-V and LLVM teach QCOS (evidence log)

Started 2026-11-01. Companion design documents: `docs/RISCV_QCOS.md` (hardware, atomic profiles, PMP, the door), `docs/LINUX_QCOS.md`.
Evidence rule as in `docs/knowledge/linux.md`: every claim carries a path and line, or a book and section, that we read; NOT FOUND means we looked.
Local clones (read-only, partial): `/home/claude/riscv/{riscv-isa-manual,riscv-opcodes,riscv-tests}`, `/home/claude/llvm/llvm-project`, `/home/claude/mit-pdos/xv6-riscv-fall19`.
A new session may need `add_repo` again.

## 1. Atomics: what the spec says and what the compiler emits

Spec (project copy `riscv-unprivileged.pdf`, 20250508, text copy line numbers):
- 13.2 Zalrsc and Listing 2 (compare-and-swap in four instructions: `lr; bne; sc; bnez`).
- 13.3, lines 4167 to 4184: the constrained LR/SC loop. At most 16 instructions placed sequentially; between `lr` and `sc` only base-I instructions without loads, stores, backward jumps, taken backward branches, JALR, FENCE, SYSTEM;
  retry code may branch backwards; `sc` has the size and address of the latest `lr`; the region must have the eventuality property (the execution environment says which). Guarantee (lines 4202 to 4210): one of four events eventually happens
  (a successful `sc`, another hart's store or AMO to the reservation set, the hart leaves the loop, the hart traps): livelock-freedom, not starvation-freedom.
- 17.1 RVWMO: every memory operation is single-copy atomic when aligned; an aligned AMO is one operation that is both load and store; a misaligned access may be split.
- A.5 (tables 89 to 92): acquire load = `l; fence r,rw`; release store = `fence rw,w; s`; fully ordered read-modify-write = `amo.aqrl` or `lr.aq; op; sc.aqrl`.

LLVM (cloned tree, newer than the clang 18.1.3 installed here, so this is the SOURCE of the behaviour, while the object code we proved comes from 18.1.3; the shapes agree):
- `llvm/lib/Target/RISCV/RISCVExpandPseudoAtomics.cpp:802` `expandAtomicCmpXchg`: the loop head is `lr.[w|d]; bne dest, cmpval, done`, the tail `sc.[w|d] scratch, newval, (addr); bnez scratch, loophead` (comments at lines 842 to 860): a constrained loop by construction.
- Same file, lines 213 to 227: for `AtomicOrdering::AcquireRelease` the load-reserved is `LR_W_AQ` (plain `LR_W` when the target has Ztso); sequentially consistent uses `LR_W_AQRL`. This is why our objects show `lr.d.aq` paired with `sc.d.rl`.
- `llvm/lib/Target/RISCV/RISCVInstrInfoA.td:203` `AMOPat` (one AMO per ordering) and `:428` to `:461` `PseudoCmpXchg32/64` (compare-exchange is a pseudo expanded after register allocation, never a library call on RV64).

Measured on our objects (clang 18.1.3, `-march=rv64imac -O2`, `tools/check_riscv.py`): acquire exchange is `amoswap.w.aq`; relaxed fetch-add is `amoadd.d zero, ...`; release store is `fence rw, w; sw`; acquire load is `ld; fence r, rw`;
strong compare-exchange is the four-instruction constrained loop, 7 such loops in the kernel, longest 4 instructions.

Findings that cost us time and are worth keeping:
- A linked image whose linker script discards `.riscv.attributes` disassembles as base ISA only: every A, M and C instruction prints as `<unknown>` unless `llvm-objdump --mattr=+m,+a,+c` is given.
- `-mstrict-align` is already clang's default for this target (objects identical with and without it) and `-mno-strict-align` changes the code. Pin the flag; the FU540 cores trap on misaligned accesses.
- An `atomic_flag` is a byte; RISC-V has no byte AMO without Zabha, so the compiler emits a word-sized `lr/sc` loop around it. Use a 32-bit word.

## 2. Privileged: PMP and Smepmp (project copy `riscv-privileged.pdf`, 20250508)

3.7 and chapter 6; the numbered equations are in `docs/DAILY_INTUITION.md` (2026-11-01, items 1 to 5) and the planned budget in `docs/RISCV_QCOS.md` section 3. Not yet checked against the `riscv-isa-manual` clone: `src/priv/smepmp.adoc` was read earlier, line numbers not recorded.

## 3. Tools we will use next (located, not yet used)

- `riscv-opcodes/extensions/rv_i` (76 lines, others `rv_m`, `rv_a`, `rv_c` and the rv32/rv64 variants): machine-readable encodings, e.g. `jal rd jimm20  6..2=0x1b 1..0=3`. Plan: generate the interpreter's decode table and the direct emitter's bit masks from these files with a Python script (frontend side), so the C17 backend contains no hand-typed opcode; the files are the independent oracle.
- `riscv-tests/isa/{rv64ui, rv64um, rv64ua, rv64uc, ...}` with `env/p/riscv_test.h` (`tohost` is the pass/fail word): conformance programs for an RV64IMAC interpreter. They need a RISC-V toolchain to build (clang can target riscv64; check that `ld.lld` links them with `link.ld` at `0x80000000`).
- Not yet read: LLVM LangRef atomics, `APInt` division (Knuth D, relevant to 128/64 division for `cos_rat`), libFuzzer, MLIR progressive lowering (the translation ladder), AMDGPU/NVPTX backends (the GPU path).

## 4. Books added to the project on 2026-11-01 (to read before the interpreter and emitter work)

`HandP_RISCV(1..3)` (Patterson and Hennessy, RISC-V edition), `Digital_Design_and_Computer_Architecture_RISC (Sarah_Harris)`, `students digital guide - mit computer structure`,
`Introduction to Assembly Language Programming`, `Programming_from_the_Ground_Up`. They match the project instructions' compiler backend (MIR nodes to raw 32-bit instruction words, reset vector, flat image `qcos_kernel.bin`)
and the CPU simulator. Reading log: `docs/DAILY_INTUITION.md`.
