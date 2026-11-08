#!/usr/bin/env python3
"""Builds the sovereign kernel (core/c17/*.c) for RISC-V 64 with LLVM (clang + lld) and proves, from the object code itself, that it fits the
profile of the "+1" monitor core: an RV64IMAC core without floating point (SiFive E51 class: rv64imac, lp64 ABI).

    python3 tools/check_riscv.py          standalone
    (also called by tools/check_c17.py)

What is proven, per object, by reading the disassembly (not the sources):
  build       every kernel source compiles for riscv64 (-march=rv64imac -mabi=lp64 -mcmodel=medany, freestanding) with -Werror
  no float    no F/D/V instruction, no compressed float instruction: the kernel cannot touch a floating point register
  no system   no CSR access, no ecall/ebreak/mret/sret/wfi/sfence/unimp: the kernel is plain unprivileged code, so it can run in M-mode (monitor
              core), S-mode or U-mode (under Linux) unchanged, and it cannot change the machine state behind the monitor's back
  atomics     the lock-free pool compiles to inline lr.d/sc.d (or amo) with fences, and nothing imports an __atomic_* library routine
  loops       every lr..sc loop in a no-gc audit image obeys the constrained-loop rules of RISC-V unprivileged manual 13.3 (at most 16
              instructions, only base-I between lr and sc, no load/store/fence/jalr/backward jump there, same size and address register),
              which is what turns "lock-free" into the manual's livelock-freedom guarantee
  strict      -mstrict-align is pinned: the FU540 cores trap on a misaligned access, so no object may rely on one
  imports     objects import only memcpy/memmove/memset/memcmp/strlen (a freestanding image supplies them) and each other
  no state    no .data/.bss/.tdata section: all state lives in memory the caller hands in
  link        all objects plus a 40-line freestanding libc link with lld, --no-undefined, --gc-sections, at the E51's RAM base
  E51 profile the same sources built with -DCOS_SINGLE_HART=1 (core/c17/cos_atomic.h) contain NO lr, sc, amo or fence instruction at all
              (the E51 has no data cache: lr/sc fault on its DTIM and uncached regions, FU540-C000 manual 3.5), and still link
It needs clang (riscv64 target), ld.lld and the llvm-objdump/llvm-nm/llvm-size tools; if any is missing the check FAILS (never silently skipped)."""
import glob
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C17 = os.path.join(ROOT, "core", "c17")
RV = os.path.join(ROOT, "core", "tests", "rv64")
OUT = os.path.join(ROOT, "build_c17", "rv64")

# the monitor-core profile: RV64 integer base, multiply/divide, atomics, compressed; no F, no D, no V; LP64 ABI; position-dependent medany code model
# -msmall-data-limit=0: LLVM would otherwise put its 8-byte constant-pool entries (division magic numbers) in .sdata, which is marked writable;
# with 0 they go to read-only .rodata.cst8, so the image has no writable section from the kernel at all
# -mstrict-align: the FU540-C000 manual says misaligned accesses trap on both the E51 and the U54, so the compiler must never emit one. Verified 2026-11-01: with
# the flag every object is byte-identical to the build without it (strict alignment is already clang's default here), while -mno-strict-align
# changes the code; pinning the flag keeps it that way if a future toolchain changes its default.
CFLAGS = ["--target=riscv64-unknown-elf", "-march=rv64imac", "-mabi=lp64", "-mcmodel=medany", "-msmall-data-limit=0", "-mstrict-align", "-ffreestanding", "-std=c17", "-O2", "-Wall", "-Wextra",
          "-Wpedantic", "-Werror", "-Wconversion", "-Wshadow", "-Wstrict-prototypes", "-Wmissing-prototypes", "-Wcast-qual", "-I" + C17,
          "-isystem", os.path.join(RV, "include")]  # a freestanding target has no <string.h>: ours declares the five routines the kernel imports
ALLOWED_UNDEFINED = {"memcpy", "memmove", "memset", "memcmp", "strlen"}
TOOLS = ["clang", "ld.lld", "llvm-objdump", "llvm-nm", "llvm-size", "llvm-readelf"]

# mnemonic classes that must never appear
FORBIDDEN = [
    (re.compile(r"^f(?!ence)"), "floating point"),
    (re.compile(r"^c\.f"), "compressed floating point"),
    (re.compile(r"^v[a-z]"), "vector"),
    (re.compile(r"^(csr|c?\.?ecall|c?\.?ebreak|mret|sret|uret|wfi|sfence|hfence|unimp|c\.unimp|dret)"), "privileged, CSR or trapping"),
]


def run(cmd, **kw):
    kw.setdefault("capture_output", True)
    kw.setdefault("text", True)
    kw.setdefault("timeout", 600)
    return subprocess.run(cmd, **kw)


def mnemonics(obj):
    p = run(["llvm-objdump", "-d", "--no-show-raw-insn", obj])
    out = []
    for line in p.stdout.splitlines():
        m = re.match(r"^\s+[0-9a-f]+:\s+(\S+)", line)
        if m:
            out.append(m.group(1))
    return out


def section_sizes(obj):
    sizes = {}
    for ln in run(["llvm-size", "-A", obj]).stdout.splitlines():
        f = ln.split()
        if len(f) >= 2 and f[0].startswith(".") and f[1].isdigit():
            sizes[f[0]] = int(f[1])
    return sizes


def run_checks(record):
    missing = [t for t in TOOLS if run(["which", t]).returncode != 0]
    if missing:
        record(False, "riscv: tools missing: " + ", ".join(missing), "the RISC-V proof needs clang with the riscv64 target, lld and the llvm binutils")
        return
    os.makedirs(OUT, exist_ok=True)
    srcs = sorted(glob.glob(os.path.join(C17, "*.c")))
    objs, errors = [], []
    for s in srcs:
        o = os.path.join(OUT, os.path.basename(s)[:-2] + ".o")
        p = run(["clang"] + CFLAGS + ["-c", s, "-o", o])
        if p.returncode != 0:
            errors.append("%s\n%s" % (os.path.basename(s), p.stderr[:1200]))
        else:
            objs.append(o)
    record(not errors, "riscv: %d kernel sources build for rv64imac/lp64 with clang (-Werror, -Wconversion, freestanding)" % len(srcs), "\n".join(errors))
    if errors:
        return

    bad, allmn, per_obj = [], {}, {}
    for o in objs:
        mn = mnemonics(o)
        per_obj[os.path.basename(o)] = mn
        for m in mn:
            allmn[m] = allmn.get(m, 0) + 1
            for rx, why in FORBIDDEN:
                if rx.search(m):
                    bad.append("%s: %s instruction '%s'" % (os.path.basename(o), why, m))
    total = sum(allmn.values())
    nofloat = [b for b in bad if "floating" in b or "vector" in b]
    nosys = [b for b in bad if b not in nofloat]
    record(total > 1000 and not nofloat, "riscv: no floating point or vector instruction in %d instructions of object code (%d distinct mnemonics)" % (total, len(allmn)),
           "\n".join(nofloat[:20]))
    record(not nosys, "riscv: no CSR access, ecall, ebreak, mret, wfi or other privileged instruction: the kernel is plain unprivileged code, valid in M, S and U mode",
           "\n".join(nosys[:20]))

    # atomics: the lock-free pool must use the hardware's own instructions
    pool = per_obj.get("cos_pool.o", [])
    lrsc = [m for m in pool if m.startswith(("lr.d", "sc.d"))]
    amo = [m for m in pool if m.startswith("amo")]
    fences = [m for m in pool if m.startswith("fence")]
    und = {}
    sizes_total = {}
    state = []
    for o in objs:
        for ln in run(["llvm-nm", "-u", o]).stdout.splitlines():
            name = ln.split()[-1] if ln.split() else ""
            if name:
                und.setdefault(name, []).append(os.path.basename(o))
        for sec, n in section_sizes(o).items():
            sizes_total[sec] = sizes_total.get(sec, 0) + n
            if n > 0 and re.match(r"^\.(data|bss|tbss|tdata|sdata|sbss)(\.|$)", sec):
                state.append("%s: section %s has %d bytes" % (os.path.basename(o), sec, n))
    defined = set()
    for o in objs:
        for ln in run(["llvm-nm", "--defined-only", o]).stdout.splitlines():
            parts = ln.split()
            if len(parts) == 3:
                defined.add(parts[2])
    atomic_calls = sorted(n for n in und if n.startswith("__atomic") or n.startswith("__sync"))
    record(bool(lrsc or amo) and bool(fences) and not atomic_calls,
           "riscv: lock-free pool compiles to inline atomics (%d lr.d/sc.d, %d amo*, %d fence) and calls no __atomic_* routine" % (len(lrsc), len(amo), len(fences)),
           "atomic library calls: %s" % atomic_calls)
    imports = ["%s imports %s" % (", ".join(sorted(set(u))), n) for n, u in sorted(und.items()) if n not in defined and n not in ALLOWED_UNDEFINED]
    record(not imports, "riscv: objects import only memcpy/memmove/memset/memcmp/strlen and each other", "\n".join(imports))
    record(not state, "riscv: no .data/.bss/.tdata section in any object: all state lives in memory the caller hands in", "\n".join(state))

    # link a freestanding image: kernel objects + the tiny libc + a conformance entry, at the E51's RAM base
    extra = []
    for s in sorted(glob.glob(os.path.join(RV, "*.c"))) + sorted(glob.glob(os.path.join(RV, "*.S"))):
        o = os.path.join(OUT, "rt_" + os.path.basename(s) + ".o")
        extra_flags = ["-fno-builtin", "-Wno-missing-prototypes"] if s.endswith(".c") else []
        p = run(["clang"] + [f for f in CFLAGS if f not in ("-Wconversion",)] + extra_flags + ["-c", s, "-o", o])
        if p.returncode != 0:
            record(False, "riscv: runtime stub %s builds" % os.path.basename(s), p.stderr[:1200])
            return
        extra.append(o)
    ld = os.path.join(RV, "link.ld")
    if not extra or not os.path.exists(ld):
        record(False, "riscv: link", "core/tests/rv64/ has no runtime or no link.ld yet")
        return
    elf = os.path.join(OUT, "kernel_probe.elf")
    p = run(["ld.lld", "-T", ld, "--no-undefined", "--gc-sections", "-o", elf] + objs + extra + ["-Map=" + os.path.join(OUT, "kernel_probe.map")])
    record(p.returncode == 0, "riscv: kernel + libc stub link with lld (--no-undefined --gc-sections) at the E51 RAM base", p.stderr[:1500])
    if p.returncode == 0:
        sz = run(["llvm-size", elf]).stdout.strip().splitlines()
        print("      info: image (%s)" % (sz[-1].split()[:3] if sz else "?"), flush=True)
    print("      info: object code .text %d bytes, .rodata* %d bytes in all kernel objects (rv64imac)" % (
        sizes_total.get(".text", 0), sum(n for s, n in sizes_total.items() if s.startswith(".rodata"))), flush=True)
    print("      info: instruction mix: " + " ".join("%s=%d" % (m, n) for m, n in sorted(allmn.items(), key=lambda kv: -kv[1])[:14]), flush=True)
    check_constrained_loops(record, objs, extra, ld)
    check_single_hart(record, srcs, extra, ld, sizes_total.get(".text", 0), objs)


# ---------------------------------------------------------------- constrained LR/SC loops
# RISC-V unprivileged manual (ratified 20250508), 13.3 "Eventual Success of Store-Conditional Instructions": a loop is "constrained" when it is at most
# 16 instructions placed sequentially, holds only an LR/SC sequence plus the code that retries it, and between the LR and the SC (and in the retry
# code) uses only base-I instructions without loads, stores, backward jumps, taken backward branches, JALR, FENCE or SYSTEM (compressed forms allowed);
# the SC has the size and address of the latest LR. Then the platform must guarantee that some hart eventually leaves the loop: livelock-freedom.
# An unconstrained sequence may never succeed on some implementations. Our "lock-free" claim for many harts therefore rests on this check.
INSN_LINE = re.compile(r"^\s*([0-9a-f]+):\s+(\S+)\s*(.*?)\s*$")
FUNC_LINE = re.compile(r"^([0-9a-f]+) <([^>]+)>:\s*$")
# a whitelist (an unknown mnemonic fails closed). Multiply and divide are the M extension, not base I, so they are not here.
BASE_I_PLAIN = set(("lui auipc addi slti sltiu xori ori andi slli srli srai add sub sll slt sltu xor srl sra or and addiw slliw srliw sraiw addw subw "
                    "sllw srlw sraw nop li mv not neg negw sext.w seqz snez sltz sgtz zext.b c.addi c.addiw c.li c.lui c.addi16sp c.addi4spn c.slli "
                    "c.srli c.srai c.andi c.mv c.add c.and c.or c.xor c.sub c.addw c.subw c.nop").split())
COND_BRANCH = set("beq bne blt bge bltu bgeu beqz bnez bltz bgez blez bgtz bgt ble bgtu bleu c.beqz c.bnez".split())
PLAIN_JUMP = {"j", "c.j"}  # no link register: a jump, not a call
LOOP_MAX = 16
RETRY_WINDOW = 4  # the retry branch must follow the sc within this many instructions (LLVM emits it right after)


def parse_image(elf):
    """llvm-objdump of a linked image -> {function: [(address, mnemonic, operand text), ...]} in address order.
    --mattr is required: link.ld discards .riscv.attributes, and without it the disassembler assumes the base ISA only and prints every
    A, M and C instruction as <unknown> (found 2026-11-01: a plain objdump of the image shows no lr, sc or amo at all).
    Compiler labels (.LBB..., .Lpcrel_hi...) are not function boundaries: an lr and its sc may sit on either side of one."""
    funcs, cur = {}, None
    for line in run(["llvm-objdump", "-d", "--no-show-raw-insn", "--mattr=+m,+a,+c", elf]).stdout.splitlines():
        m = FUNC_LINE.match(line)
        if m:
            if not m.group(2).startswith(".L") or cur is None:
                cur = funcs.setdefault(m.group(2), [])
            continue
        m = INSN_LINE.match(line)
        if m and cur is not None:
            cur.append((int(m.group(1), 16), m.group(2), re.sub(r"\s*<[^>]*>", "", m.group(3))))
    return funcs


def target_of(ops):
    last = ops.split(",")[-1].strip()
    return int(last, 16) if re.fullmatch(r"0x[0-9a-f]+", last) else None


def dest_of(ops):
    return ops.split(",")[0].strip()


def check_loops(funcs):
    """-> (loops, longest, kinds, exits_back, errors). Every lr must open a constrained loop (13.3) inside its own function; every sc must close one."""
    loops, longest, kinds, errors, closed, exits_back = 0, 0, {"w": 0, "d": 0}, [], set(), 0
    for fn, ins in funcs.items():
        for i, (a, mn, ops) in enumerate(ins):
            if not mn.startswith("lr."):
                continue
            where = "%s+0x%x" % (fn, a - ins[0][0])
            size = mn.split(".")[1]
            areg = re.search(r"\((\w+)\)", ops)
            k = next((j for j in range(i + 1, len(ins)) if ins[j][1].startswith(("sc.", "lr."))), None)
            if k is None or not ins[k][1].startswith("sc.") or areg is None:
                errors.append("%s: lr is not followed by its sc in the same function" % where)
                continue
            closed.add((fn, k))
            sc_mn, sc_ops = ins[k][1], ins[k][2]
            sreg = re.search(r"\((\w+)\)", sc_ops)
            if sc_mn.split(".")[1] != size or sreg is None or sreg.group(1) != areg.group(1):
                errors.append("%s: sc (%s %s) does not match its lr (%s %s) in size or address register" % (where, sc_mn, sc_ops, mn, ops))
                continue
            retry = None
            for j in range(k + 1, min(len(ins), k + 1 + RETRY_WINDOW)):  # the code that retries the sequence
                ja, jm, jo = ins[j]
                if jm in COND_BRANCH or jm in PLAIN_JUMP:
                    if target_of(jo) == a:
                        retry = j
                        break
                elif jm not in BASE_I_PLAIN:
                    errors.append("%s: '%s %s' is not allowed in the retry code (13.3)" % (where, jm, jo))
                    break
            if retry is None:
                errors.append("%s: no branch back to the lr within %d instructions after the sc" % (where, RETRY_WINDOW))
                continue
            end = ins[retry][0]  # the loop is [a, end]
            for j in range(i + 1, k):  # between lr and sc
                ja, jm, jo = ins[j]
                tgt = target_of(jo)
                if jm in COND_BRANCH or jm in PLAIN_JUMP:
                    # forward: skips or leaves the loop. Backward is allowed only when it LEAVES the loop (target before the lr): then no sc follows on
                    # that path, so it is not an lr..sc sequence at all (13.3 speaks of the code executed between an lr and its sc), and leaving the
                    # loop is one of the events the manual's eventuality guarantee lists. A backward branch that stays inside would be an inner loop.
                    if tgt is None or not (tgt > ja or tgt < a):
                        errors.append("%s: branch '%s %s' stays inside the loop or is unresolved (a loop within lr..sc)" % (where, jm, jo))
                    elif tgt <= ja:
                        exits_back += 1
                elif jm in BASE_I_PLAIN:
                    if dest_of(jo) == areg.group(1):
                        errors.append("%s: '%s %s' overwrites the lr/sc address register between lr and sc" % (where, jm, jo))
                else:
                    errors.append("%s: '%s %s' is not allowed between lr and sc (13.3)" % (where, jm, jo))
            for ja, jm, jo in ins:  # single entry: nothing outside the loop may jump into its middle, so every path to the sc starts at the lr
                if (jm in COND_BRANCH or jm in PLAIN_JUMP) and not (a <= ja <= end):
                    tgt = target_of(jo)
                    if tgt is not None and a < tgt <= end:
                        errors.append("%s: '%s %s' at 0x%x jumps into the middle of the loop" % (where, jm, jo, ja))
            length = retry - i + 1
            if length > LOOP_MAX:
                errors.append("%s: loop is %d instructions, the limit is %d" % (where, length, LOOP_MAX))
            loops += 1
            longest = max(longest, length)
            kinds[size] = kinds.get(size, 0) + 1
    for fn, ins in funcs.items():
        for k, (a, mn, ops) in enumerate(ins):
            if mn.startswith("sc.") and (fn, k) not in closed:
                errors.append("%s+0x%x: sc without an lr that opens a loop" % (fn, a - ins[0][0]))
    return loops, longest, kinds, exits_back, errors


def selftest_loop_checker():
    """A checker that cannot fail proves nothing: feed check_loops hand-made code, one rule broken at a time. -> (rejected, errors_of_the_test)."""
    def code(*rows):
        return {"t": [(0x1000 + 4 * n, mn, ops) for n, (mn, ops) in enumerate(rows)]}

    lr, sc, retry, done = ("lr.d.aq", "a4, (a0)"), ("sc.d.rl", "a5, a3, (a0)"), ("bnez", "a5, 0x1000"), ("ret", "")
    exit_fwd = ("bne", "a4, t2, 0x1100")
    problems = []
    loops, _, _, _, errs = check_loops(code(lr, exit_fwd, sc, retry, done))
    if loops != 1 or errs:
        problems.append("the canonical loop (lr; bne; sc; bnez) was rejected: %s" % errs)
    bad = {
        "a load between lr and sc": code(lr, ("lw", "a2, 0x0(a1)"), exit_fwd, sc, retry, done),
        "a store between lr and sc": code(lr, ("sw", "a2, 0x0(a1)"), exit_fwd, sc, retry, done),
        "a fence between lr and sc": code(lr, ("fence", "rw, rw"), exit_fwd, sc, retry, done),
        "a multiply between lr and sc (M, not base I)": code(lr, ("mul", "a2, a2, a3"), exit_fwd, sc, retry, done),
        "a call between lr and sc": code(lr, ("jal", "ra, 0x2000"), exit_fwd, sc, retry, done),
        "an inner backward branch": code(lr, ("bnez", "a2, 0x1000"), exit_fwd, sc, retry, done),
        "the address register overwritten": code(lr, ("addi", "a0, a0, 8"), exit_fwd, sc, retry, done),
        "lr.d with sc.w": code(lr, exit_fwd, ("sc.w", "a5, a3, (a0)"), retry, done),
        "lr and sc on different address registers": code(lr, exit_fwd, ("sc.d.rl", "a5, a3, (a1)"), retry, done),
        "17 instructions": code(lr, *([("addi", "a2, a2, 1")] * 14), sc, retry, done),
        "no retry branch": code(lr, exit_fwd, sc, done),
        "an sc with no lr": code(("addi", "a2, a2, 1"), sc, done),
        "a jump into the middle of the loop": code(lr, exit_fwd, sc, retry, ("j", "0x1008"), done),
    }
    for what, funcs in bad.items():
        if not check_loops(funcs)[4]:
            problems.append("NOT rejected: %s" % what)
    return len(bad), problems


def check_constrained_loops(record, objs, extra, ld):
    audit = os.path.join(OUT, "kernel_audit.elf")  # NOT gc'ed: every kernel function is in the image, branch targets are resolved by the linker
    p = run(["ld.lld", "-T", ld, "--no-undefined", "-o", audit] + objs + extra)
    if p.returncode != 0:
        record(False, "riscv: audit image (all kernel code, no section gc) links", p.stderr[:1500])
        return
    n_bad, problems = selftest_loop_checker()
    record(not problems, "riscv: the constrained-loop checker accepts the canonical lr;bne;sc;bnez loop and rejects %d seeded violations of 13.3" % n_bad, "\n".join(problems))
    funcs = parse_image(audit)
    n_obj = sum(1 for o in objs for m in mnemonics(o) if m.startswith("lr."))
    n_img = sum(1 for ins in funcs.values() for _, m, _ in ins if m.startswith("lr."))
    loops, longest, kinds, exits_back, errors = check_loops(funcs)
    if loops != n_obj or n_obj != n_img:
        errors.append("coverage: %d lr in the objects, %d in the audit image, %d loops verified" % (n_obj, n_img, loops))
    record(not errors and loops > 0,
           "riscv: all %d lr..sc loops (%d lr.w, %d lr.d) obey the RISC-V 13.3 constrained-loop rules: single entry, base-I only between lr and sc, no "
           "load/store/fence/jalr and no inner loop there, same size and address, retry right after, longest %d of %d instructions; %d failed-compare exits go backwards "
           "to the caller's own retry code" % (loops, kinds.get("w", 0), kinds.get("d", 0), longest, LOOP_MAX, exits_back),
           "\n".join(errors[:25]))


# ---------------------------------------------------------------- the E51 profile: -DCOS_SINGLE_HART=1
ATOMIC_MNEMONIC = re.compile(r"^(lr\.|sc\.|amo|fence)")


def check_single_hart(record, srcs, extra, ld, text_many, objs_many):
    out = os.path.join(OUT, "single_hart")
    os.makedirs(out, exist_ok=True)
    objs, errors = [], []
    for s in srcs:
        o = os.path.join(out, os.path.basename(s)[:-2] + ".o")
        p = run(["clang"] + CFLAGS + ["-DCOS_SINGLE_HART=1", "-c", s, "-o", o])
        if p.returncode != 0:
            errors.append("%s\n%s" % (os.path.basename(s), p.stderr[:1200]))
        else:
            objs.append(o)
    record(not errors, "riscv E51 profile: %d kernel sources build with -DCOS_SINGLE_HART=1 (same flags, -Werror, -Wconversion)" % len(srcs), "\n".join(errors))
    if errors:
        return
    bad, total = [], 0
    for o in objs:
        mn = mnemonics(o)
        total += len(mn)
        for m in mn:
            if ATOMIC_MNEMONIC.match(m):
                bad.append("%s: atomic or fence instruction '%s'" % (os.path.basename(o), m))
            for rx, why in FORBIDDEN:
                if rx.search(m):
                    bad.append("%s: %s instruction '%s'" % (os.path.basename(o), why, m))
    n_many = sum(1 for o in objs_many for m in mnemonics(o) if ATOMIC_MNEMONIC.match(m))  # the detector must be alive: it sees them where they are
    record(not bad and total > 1000 and n_many > 0,
           "riscv E51 profile: no lr, sc, amo or fence instruction (and no float, vector, CSR or trap) in %d instructions, where the same detector finds %d in the "
           "many-harts objects: the monitor core executes only what its hardware can run" % (total, n_many),
           "\n".join(bad[:25]) or "the detector found no atomic instruction in the many-harts objects: it is blind")
    undefined, defined, state, sizes = {}, set(), [], {}
    for o in objs:
        for ln in run(["llvm-nm", "-u", o]).stdout.splitlines():
            if ln.split():
                undefined.setdefault(ln.split()[-1], []).append(os.path.basename(o))
        for ln in run(["llvm-nm", "--defined-only", o]).stdout.splitlines():
            parts = ln.split()
            if len(parts) == 3:
                defined.add(parts[2])
        for sec, n in section_sizes(o).items():
            sizes[sec] = sizes.get(sec, 0) + n
            if n > 0 and re.match(r"^\.(data|bss|tbss|tdata|sdata|sbss)(\.|$)", sec):
                state.append("%s: section %s has %d bytes" % (os.path.basename(o), sec, n))
    imports = ["%s imports %s" % (", ".join(sorted(set(u))), n) for n, u in sorted(undefined.items()) if n not in defined and n not in ALLOWED_UNDEFINED]
    record(not imports and not state, "riscv E51 profile: imports only memcpy/memmove/memset/memcmp/strlen and each other (no __atomic_* routine); no .data/.bss",
           "\n".join(imports + state))
    elf = os.path.join(out, "kernel_probe_e51.elf")
    p = run(["ld.lld", "-T", ld, "--no-undefined", "--gc-sections", "-o", elf] + objs + extra + ["-Map=" + os.path.join(out, "kernel_probe_e51.map")])
    record(p.returncode == 0, "riscv E51 profile: kernel + libc stub link with lld (--no-undefined --gc-sections) at the RAM base", p.stderr[:1500])
    print("      info: E51 profile .text %d bytes (many-harts profile %d bytes)" % (sizes.get(".text", 0), text_many), flush=True)


def main():
    results = []

    def record(ok, title, detail=""):
        results.append(ok)
        print("%s  %s" % ("ok  " if ok else "FAIL", title), flush=True)
        if detail and not ok:
            for line in detail.strip().splitlines()[:30]:
                print("      " + line, flush=True)

    run_checks(record)
    print("\n%s: %d of %d checks passed" % ("ALL OK" if all(results) else "FAILED", sum(results), len(results)))
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
