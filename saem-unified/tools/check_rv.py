#!/usr/bin/env python3
"""Proves the assembly layer (core/c17/cos_rv.c: encode, decode, format) against an independent oracle: LLVM's assembler and disassembler.

    python3 tools/check_rv.py            full run (about a minute)
    python3 tools/check_rv.py --quick    a tenth of the cases

Nothing here trusts our own code or our own tables:
  tables      tools/gen_rv_tab.py --check: the committed tables are exactly what the generator makes from riscv-opcodes (skipped, and said so, if the checkout is absent)
  encode      for every instruction of RV32 and of RV64: operands (every register value in every register field, immediate edges and random values, all aq/rl,
              all 256 fence combinations) -> our encoder AND llvm-mc -filetype=obj; the words must be identical
  refuse      one step beyond every range (immediates, odd branch offsets, out-of-range shifts and CSR numbers): our encoder refuses; where llvm-mc can be asked
              the same question it refuses too (differences are listed, not hidden)
  decode      a few hundred thousand words per register width (structured around every opcode, every single-bit neighbour of every opcode, a sweep of every
              opcode/funct3/funct7 combination, uniform random): our decoder and llvm-objdump agree on legal versus illegal; the mnemonic agrees; every word we
              accept returns to the same word when re-encoded; and our text, assembled by llvm-mc, gives the original word back (decode -> format -> LLVM -> word)
  scope       every word that LLVM accepts and we refuse must be an instruction outside our profile (sret, sfence.vma, ...): never one of ours
Exit code 0 only if everything passes."""
import argparse
import os
import random
import re
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C17 = os.path.join(ROOT, "core", "c17")
TESTS = os.path.join(ROOT, "core", "tests")
BUILD = os.path.join(ROOT, "build_c17")
MATTR = "+m,+a,+zicsr,+zifencei"
ALL_EXTS = 31
results = []


def record(ok, title, detail=""):
    results.append(ok)
    print("%s  %s" % ("ok  " if ok else "FAIL", title), flush=True)
    if detail and not ok:
        for ln in detail.strip().splitlines()[:30]:
            print("      " + ln, flush=True)


def run(cmd, **kw):
    kw.setdefault("capture_output", True)
    kw.setdefault("text", True)
    kw.setdefault("timeout", 900)
    return subprocess.run(cmd, **kw)


DUMP = os.path.join(BUILD, "c17_rv_dump")


def build_dump():
    os.makedirs(BUILD, exist_ok=True)
    p = run(["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Werror", "-I" + C17, os.path.join(TESTS, "c17_rv_dump.c"), os.path.join(C17, "cos_rv.c"),
             os.path.join(C17, "cos_rv_tab.c"), "-o", DUMP])
    if p.returncode != 0:
        raise SystemExit("build of c17_rv_dump failed:\n" + p.stderr)


def ours(mode, xlen, exts, lines):
    p = run([DUMP, mode, str(xlen), str(exts)], input="".join(l + "\n" for l in lines))
    out = p.stdout.splitlines()
    if p.returncode != 0 or (lines and len(out) != len(lines)):
        raise SystemExit("c17_rv_dump %s failed (rc %s, %d lines for %d)\n%s" % (mode, p.returncode, len(out), len(lines), p.stderr[:500]))
    return out


def table_rows():
    rows = []
    for ln in run([DUMP, "rows", "32", "31"]).stdout.splitlines():
        n, cls, match, mask, xl, ext = ln.split()
        rows.append(dict(name=n, cls=cls, match=int(match, 16), mask=int(mask, 16), xlens=int(xl), ext=int(ext)))
    return rows


def llvm_assemble(tmp, lines, xlen):
    """-> (words or None, set of 1-based line numbers with an assembler error)."""
    s, o, b = [os.path.join(tmp, x) for x in ("a.s", "a.o", "a.bin")]
    with open(s, "w") as f:
        f.write("".join(l + "\n" for l in lines))
    p = run(["llvm-mc", "-filetype=obj", "-triple=riscv%d" % xlen, "-mattr=" + MATTR + ",-relax", "-o", o, s])
    errs = {int(m.group(1)) for m in re.finditer(r"a\.s:(\d+):\d+: error", p.stderr)}
    if p.returncode != 0:
        return None, errs
    p = run(["llvm-objcopy", "-O", "binary", "--only-section=.text", o, b])
    data = open(b, "rb").read()
    return list(struct.unpack("<%dI" % (len(data) // 4), data)), errs


def llvm_disassemble(tmp, words, xlen):
    """-> list of text ('<unknown>' for an illegal word); every word must start at its own 4-byte address."""
    b, o = os.path.join(tmp, "d.bin"), os.path.join(tmp, "d.o")
    open(b, "wb").write(struct.pack("<%dI" % len(words), *words))
    run(["llvm-objcopy", "-I", "binary", "-O", "elf%d-littleriscv" % xlen, "--rename-section", ".data=.text,alloc,load,readonly,code", b, o])
    p = run(["llvm-objdump", "-d", "-M", "no-aliases", "--mattr=" + MATTR, o])
    out, rx = [], re.compile(r"^\s+([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)\s*(.*)$")
    for ln in p.stdout.splitlines():
        m = rx.match(ln)
        if m:
            if int(m.group(1), 16) != 4 * len(out):
                k = len(out)
                raise SystemExit("objdump alignment lost at 0x%s (word %d): previous words %s, line: %s" % (
                    m.group(1), k, ["0x%08x" % w for w in words[max(0, k - 2):k + 1]], ln.strip()))
            out.append(" ".join(m.group(3).split()))
    if len(out) != len(words):
        raise SystemExit("objdump gave %d lines for %d words" % (len(out), len(words)))
    return out


# ------------------------------------------------------------------ operands, written from the specification and independent of cos_rv.c
def edges(lo, hi, step=1):
    pts = {lo, lo + step, -step if lo < 0 else 0, 0, step, 2 * step, hi - step, hi, 15 * step, 16 * step, 255 * step, 256 * step, 1023 * step, 1024 * step}
    return sorted(v for v in pts if lo <= v <= hi and v % step == 0)


def imm_set(cls, rng, n):
    if cls in ("I", "L", "JR", "S"):
        v = edges(-2048, 2047) + [rng.randrange(-2048, 2048) for _ in range(n)]
    elif cls == "SH5":
        v = list(range(32))
    elif cls == "SH6":
        v = list(range(64))
    elif cls == "B":
        v = edges(-4096, 4094, 2) + [rng.randrange(-2048, 2048) * 2 for _ in range(n)]
    elif cls == "U":
        v = [0, 1, 2, 0x7FFFF, 0x80000, 0xFFFFF] + [rng.randrange(0, 1 << 20) for _ in range(n)]
    elif cls == "J":
        v = edges(-1048576, 1048574, 2) + [rng.randrange(-524288, 524288) * 2 for _ in range(n)]
    elif cls in ("CSR", "CSRI"):
        v = [0, 1, 0x300, 0x305, 0x340, 0x341, 0x7B0, 0xB00, 0xC00, 0xF11, 0xF14, 0xFFF] + [rng.randrange(0, 4096) for _ in range(n)]
    elif cls == "FENCE":
        v = [(p << 4) | s for p in range(16) for s in range(16)] + [0x833]
    else:
        v = [0]
    return sorted(set(v))


USES = {"R": ("rd", "rs1", "rs2"), "I": ("rd", "rs1"), "L": ("rd", "rs1"), "JR": ("rd", "rs1"), "SH5": ("rd", "rs1"), "SH6": ("rd", "rs1"),
        "S": ("rs1", "rs2"), "B": ("rs1", "rs2"), "U": ("rd",), "J": ("rd",), "AMO": ("rd", "rs1", "rs2"), "LR": ("rd", "rs1"),
        "CSR": ("rd", "rs1"), "CSRI": ("rd", "rs1"), "FENCE": (), "FENCEI": (), "N": ()}


def cases(cls, rng, n):
    """(rd, rs1, rs2, aq, rl, imm) tuples: every register value in every register field the class uses, and every immediate edge."""
    imms = imm_set(cls, rng, n)
    mid = imms[len(imms) // 2]
    base = {"rd": 7, "rs1": 14, "rs2": 21}
    out = []
    if not USES[cls]:  # fence, fence.i, ecall ...: no register operand, the reserved fields stay zero
        return [(0, 0, 0, 0, 0, im) for im in imms]
    for f in USES[cls]:
        for v in range(32):
            d = dict(base)
            d[f] = v
            out.append((d["rd"], d["rs1"], d["rs2"], 0, 0, mid))
    for im in imms:
        out.append((rng.randrange(32), rng.randrange(32), rng.randrange(32), 0, 0, im))
    if cls in ("AMO", "LR"):
        out = [(rd, rs1, rs2, aq, rl, 0) for (rd, rs1, rs2, _, _, _) in out[:40] for aq in (0, 1) for rl in (0, 1)]
    return out


def rel(imm):
    return ".%s%d" % ("-" if imm < 0 else "+", abs(imm))


def fence_arg(a):
    return "0" if a == 0 else "".join(c for c, bit in (("i", 8), ("o", 4), ("r", 2), ("w", 1)) if a & bit)


def asm_text(name, cls, rd, rs1, rs2, aq, rl, imm):
    x = lambda r: "x%d" % r
    suffix = ".aqrl" if aq and rl else ".aq" if aq else ".rl" if rl else ""
    if cls == "R":
        return "%s %s, %s, %s" % (name, x(rd), x(rs1), x(rs2))
    if cls in ("I", "SH5", "SH6"):
        return "%s %s, %s, %d" % (name, x(rd), x(rs1), imm)
    if cls in ("L", "JR"):
        return "%s %s, %d(%s)" % (name, x(rd), imm, x(rs1))
    if cls == "S":
        return "%s %s, %d(%s)" % (name, x(rs2), imm, x(rs1))
    if cls == "B":
        return "%s %s, %s, %s" % (name, x(rs1), x(rs2), rel(imm))
    if cls == "U":
        return "%s %s, %d" % (name, x(rd), imm)
    if cls == "J":
        return "%s %s, %s" % (name, x(rd), rel(imm))
    if cls == "AMO":
        return "%s%s %s, %s, (%s)" % (name, suffix, x(rd), x(rs2), x(rs1))
    if cls == "LR":
        return "%s%s %s, (%s)" % (name, suffix, x(rd), x(rs1))
    if cls == "CSR":
        return "%s %s, %d, %s" % (name, x(rd), imm, x(rs1))
    if cls == "CSRI":
        return "%s %s, %d, %d" % (name, x(rd), imm, rs1)
    if cls == "FENCE":
        return "fence.tso" if imm == 0x833 else "fence %s, %s" % (fence_arg(imm >> 4 & 15), fence_arg(imm & 15))
    return name  # FENCEI, N


def names_for(rows, xlen, exts=ALL_EXTS):
    return [r for r in rows if r["xlens"] & (1 if xlen == 32 else 2) and (r["ext"] == 0 or r["ext"] & exts)]


# ------------------------------------------------------------------ the checks
def check_tables():
    gen = os.path.join(ROOT, "tools", "gen_rv_tab.py")
    p = run([sys.executable, gen, "--check"])
    if p.returncode == 2:
        print("skip  tables: riscv-opcodes is not checked out here (%s); the committed tables are used as they are" % p.stderr.strip()[:90], flush=True)
        return
    record(p.returncode == 0, "tables: " + p.stdout.strip(), p.stdout + p.stderr)


def check_encode(tmp, rows, xlen, rng, n):
    lines_asm, lines_c, who = [], [], []
    for r in names_for(rows, xlen):
        for (rd, rs1, rs2, aq, rl, imm) in cases(r["cls"], rng, n):
            lines_asm.append(asm_text(r["name"], r["cls"], rd, rs1, rs2, aq, rl, imm))
            lines_c.append("%s %d %d %d %d %d %d" % (r["name"], rd, rs1, rs2, aq, rl, imm))
    want, errs = llvm_assemble(tmp, lines_asm, xlen)
    if want is None or len(want) != len(lines_asm):
        record(False, "encode RV%d: llvm-mc refused some of our test lines" % xlen, "lines with errors: %s\n" % sorted(errs)[:10] +
               "\n".join(lines_asm[i - 1] for i in sorted(errs)[:10]))
        return
    got = ours("enc", xlen, ALL_EXTS, lines_c)
    bad = [(a, g, w) for a, g, w in zip(lines_asm, got, want) if g != "OK 0x%08x" % w]
    record(not bad, "encode RV%d: %d instruction words equal llvm-mc's (%d instructions, every register value, immediate edges, all aq/rl, all fences)" % (
        xlen, len(want), len(names_for(rows, xlen))), "\n".join("%s: ours %s, llvm 0x%08x" % b for b in bad[:10]))


def refusal_lines(r, xlen):
    """(kind, asm text, c line, why) for operands one step outside the legal range.  kind: parse = llvm-mc refuses while reading the line; fixup = it refuses when it
    lays the instruction out; relaxed = LLVM does not refuse, it rewrites an out-of-range conditional branch into a two-instruction sequence (we refuse: one word only)."""
    cls, n = r["cls"], r["name"]
    out = []
    if cls in ("I", "L", "JR", "S"):
        out += [("parse", asm_text(n, cls, 5, 6, 7, 0, 0, v), "%s 5 6 7 0 0 %d" % (n, v), "imm %d" % v) for v in (-2049, 2048)]
    elif cls in ("SH5", "SH6"):
        top = 32 if cls == "SH5" else 64
        out += [("parse", asm_text(n, cls, 5, 6, 0, 0, 0, v), "%s 5 6 0 0 0 %d" % (n, v), "shamt %d" % v) for v in (-1, top)]
    elif cls == "B":
        out += [("relaxed", asm_text(n, cls, 0, 6, 7, 0, 0, v), "%s 0 6 7 0 0 %d" % (n, v), "offset %d" % v) for v in (-4098, 4096)]
        out += [("fixup", asm_text(n, cls, 0, 6, 7, 0, 0, v), "%s 0 6 7 0 0 %d" % (n, v), "offset %d" % v) for v in (1, 3, -3)]
    elif cls == "J":
        out += [("fixup", asm_text(n, cls, 1, 0, 0, 0, 0, v), "%s 1 0 0 0 0 %d" % (n, v), "offset %d" % v) for v in (-1048578, 1048576, 1, -1)]
    elif cls == "U":
        out += [("parse", asm_text(n, cls, 5, 0, 0, 0, 0, v), "%s 5 0 0 0 0 %d" % (n, v), "imm20 %d" % v) for v in (-1, 1048576)]
    elif cls in ("CSR", "CSRI"):
        out += [("parse", asm_text(n, cls, 5, 6, 0, 0, 0, v), "%s 5 6 0 0 0 %d" % (n, v), "csr %d" % v) for v in (-1, 4096)]
    return out


def check_refusals(tmp, rows, xlen):
    kinds, c_lines, asm_lines, why = [], [], [], []
    for r in names_for(rows, xlen):
        for k, a, c, w in refusal_lines(r, xlen):
            kinds.append(k), asm_lines.append(a), c_lines.append(c), why.append("%s: %s" % (r["name"], w))
    got = ours("enc", xlen, ALL_EXTS, c_lines)
    accepted = [(w, g) for w, g in zip(why, got) if not g.startswith("ERR range")]
    # register 32, aq/rl 2, an instruction of the other width, an extension that is off
    extra = ours("enc", xlen, ALL_EXTS, ["add 32 1 2 0 0 0", "add 1 32 2 0 0 0", "add 1 2 32 0 0 0", "amoadd.w 1 2 3 2 0 0", "lw 1 32 0 0 0 0", "lui 32 0 0 0 0 0"])
    extra_ok = [e.startswith("ERR invalid") for e in extra]
    off = ours("enc", xlen, 0, ["add 1 2 3 0 0 0", "mul 1 2 3 0 0 0", "csrrw 1 2 0 0 0 773", "mret 0 0 0 0 0 0", "lr.w 1 2 0 0 0 0", "fence.i 0 0 0 0 0 0"])
    off_ok = [off[0].startswith("OK")] + [o.startswith("ERR not_found") for o in off[1:]]
    nf = ours("enc", 32, ALL_EXTS, ["addiw 1 2 0 0 0 0", "ld 1 2 0 0 0 0", "lr.d 1 2 0 0 0 0"])
    nf_ok = all(e.startswith("ERR not_found") for e in nf) if xlen == 32 else True
    record(not accepted and all(extra_ok) and all(off_ok) and nf_ok,
           "refuse RV%d: %d operands one step outside their range are refused (immediates, odd or out-of-range branch/jump offsets, shifts, CSR numbers, imm20); "
           "register 32, aq/rl 2, RV64-only instructions in RV32 and instructions of a switched-off extension are refused" % (xlen, len(c_lines)),
           "accepted: %s\nextra %s off %s nf %s" % (accepted[:8], extra, off, nf))
    # the same operands through LLVM, in two groups: errors found while parsing hide errors found while laying out
    notes = []
    for kind in ("parse", "fixup"):
        idx = [i for i, k in enumerate(kinds) if k == kind]
        _, errs = llvm_assemble(tmp, [asm_lines[i] for i in idx], xlen)
        missed = [why[idx[j - 1]] for j in sorted(set(range(1, len(idx) + 1)) - errs)]
        record(not missed, "refuse RV%d vs LLVM (%s): llvm-mc refuses all %d of those operands too" % (xlen, kind, len(idx)), "llvm-mc ACCEPTS: " + "; ".join(missed[:12]))
    n_relaxed = sum(1 for k in kinds if k == "relaxed")
    notes.append("%d out-of-range conditional-branch offsets are not compared: LLVM rewrites them into a two-instruction sequence" % n_relaxed)
    print("      info: " + "; ".join(notes), flush=True)


def make_words(rows, xlen, rng, scale):
    mine = names_for(rows, xlen)
    words = []
    for r in mine:  # structured: opcode bits fixed, every other bit random; and each single-bit neighbour of the opcode bits
        free = 0xFFFFFFFF & ~r["mask"]
        for _ in range(int(120 * scale)):
            words.append(r["match"] | (rng.getrandbits(32) & free))
        for bit in range(32):
            if r["mask"] >> bit & 1:
                for _ in range(max(1, int(2 * scale))):
                    words.append((r["match"] ^ (1 << bit)) | (rng.getrandbits(32) & free))
    fills = [0, 0x01FF8F80, rng.getrandbits(32) & 0x01FF8F80, rng.getrandbits(32) & 0x01FF8F80]
    for op5 in range(32):  # every opcode / funct3 / funct7 combination with four operand patterns
        for f3 in range(8):
            for f7 in range(128) if scale >= 1 else range(0, 128, 9):
                for fill in fills:
                    words.append(f7 << 25 | f3 << 12 | op5 << 2 | 3 | fill)
    for _ in range(int(150000 * scale)):
        words.append(rng.getrandbits(32) | 3)
    for w in (0xFFFFFFFF, 0x00000003, 0x7FFFFFFF, 0x80000003, 0x00000073, 0x00100073, 0x30200073, 0x10500073, 0x10200073, 0x12000073):
        words.append(w)
    return sorted(set(words))


def check_decode(tmp, rows, xlen, rng, scale):
    words = make_words(rows, xlen, rng, scale)
    longs = [w for w in words if w & 3 != 3 or w & 0x1C == 0x1C]  # 16-bit forms (C extension) and the reserved 48-bit-and-longer formats: we refuse them (documented);
    words = [w for w in words if w & 3 == 3 and w & 0x1C != 0x1C]  # objdump walks over them in steps of 2 to 8 bytes, so they cannot be lined up with the 4-byte words
    refused = ours("dec", xlen, ALL_EXTS, ["0x%08x" % w for w in longs])
    record(all(x == "ILLEGAL" for x in refused), "decode RV%d: %d words that are not 32-bit instructions (16-bit forms, reserved long formats) are all refused" % (xlen, len(longs)),
           "accepted: %s" % [("0x%08x" % w, x) for w, x in zip(longs, refused) if x != "ILLEGAL"][:5])
    mine = ours("dec", xlen, ALL_EXTS, ["0x%08x" % w for w in words])
    theirs = llvm_disassemble(tmp, words, xlen)
    our_names = {r["name"] for r in names_for(rows, xlen)}
    mism, scope, accepted, rt_words, rt_lines, rt_src = [], 0, 0, [], [], []
    noncanon, lenient = 0, []
    for w, o, t in zip(words, mine, theirs):
        legal_llvm = t != "<unknown>"
        if o.startswith("OK"):
            accepted += 1
            _, canon, text = o.split(" ", 2)
            if canon == "0":
                noncanon += 1  # a reserved field is set (fence, fence.i): the specification says to execute it, LLVM refuses it; we flag it for the verifier
                continue
            if not legal_llvm:
                mism.append("0x%08x: we decode '%s', LLVM says illegal" % (w, text))
            elif text.split()[0] != ("csrrw" if w == 0xC0001073 and t.split()[0] == "unimp" else t.split()[0]):
                # (LLVM has a real instruction `unimp` for the word csrrw x0, cycle, x0 = 0xC0001073, the canonical illegal instruction of the assembly manual; same word, other name)
                mism.append("0x%08x: mnemonic %s vs LLVM %s" % (w, text.split()[0], t.split()[0]))
            else:
                rt_words.append(w), rt_lines.append(text)
        elif o == "ILLEGAL":
            if legal_llvm:
                base = re.sub(r"\.(aq|rl|aqrl)$", "", t.split()[0])
                if xlen == 32 and base in ("slli", "srli", "srai") and w & 0x7F == 0x13 and (w >> 12) & 7 in (1, 5) and (w >> 25) & 1:
                    # RV32 shift with imm[5] = 1. LLVM 18.1.3 takes a 6-bit shift amount in RV32 too (`srli s7, a2, 62`), but the specification gives the RV32I shifts
                    # imm[11:5] = 0000000 (srai 0100000), so such a word is no RV32I instruction (riscv-unprivileged 2.4.1 encoding table; for the same encoding space the
                    # bit-manipulation chapters say "For RV32, the encodings corresponding to shamt[5]=1 are reserved"). We refuse it. A deviation of the ORACLE,
                    # counted and printed in the result line, never silently skipped.
                    lenient.append(w)
                elif base in our_names or t.split()[0] in our_names:
                    mism.append("0x%08x: LLVM decodes '%s', we say illegal" % (w, t))
                else:
                    scope += 1
        else:
            mism.append("0x%08x: %s" % (w, o))
    record(not mism, "decode RV%d: %d words: %d accepted by us, legal/illegal and mnemonic agree with llvm-objdump on all; %d accepted with a set reserved field "
           "(not assemblable); %d words LLVM accepts are outside our profile (sret, sfence.vma, ...)%s" % (
               xlen, len(words), accepted, noncanon, scope,
               "; %d RV32 shifts with imm[5]=1 that LLVM 18 accepts and the specification reserves (we refuse them)" % len(lenient) if xlen == 32 else ""),
           "\n".join(mism[:12]) + "\n(%d mismatches)" % len(mism))
    back, errs = llvm_assemble(tmp, rt_lines, xlen)
    bad = []
    if back is None or len(back) != len(rt_words):
        bad = ["llvm-mc refused our text on lines %s: %s" % (sorted(errs)[:6], [rt_lines[i - 1] for i in sorted(errs)[:6]])]
    else:
        bad = ["0x%08x -> '%s' -> 0x%08x" % (w, t, b) for w, t, b in zip(rt_words, rt_lines, back) if w != b]
    record(not bad, "round trip RV%d: decode -> our text -> llvm-mc -> the original word, for %d words" % (xlen, len(rt_words)), "\n".join(bad[:10]))


def run_all(scale):
    build_dump()
    rows = table_rows()
    check_tables()
    rng = random.Random(20261102)
    with tempfile.TemporaryDirectory() as tmp:
        for xlen in (32, 64):
            check_encode(tmp, rows, xlen, rng, max(4, int(40 * scale)))
            check_refusals(tmp, rows, xlen)
            check_decode(tmp, rows, xlen, rng, scale)


def run_checks(rec, quick=False):
    """Entry point for tools/check_c17.py: the same proofs, reported through its record(ok, title, detail)."""
    global record
    missing = [t for t in ("llvm-mc", "llvm-objdump", "llvm-objcopy") if run(["which", t]).returncode != 0]
    if missing:
        rec(False, "rv: oracle tools missing: " + ", ".join(missing), "the assembly-layer proof needs llvm-mc, llvm-objdump and llvm-objcopy")
        return
    saved = record
    record = lambda ok, title, detail="": rec(ok, "rv " + title, detail)  # noqa: E731
    try:
        run_all(0.1 if quick else 1.0)
    finally:
        record = saved


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--quick", action="store_true")
    a = ap.parse_args()
    run_all(0.1 if a.quick else 1.0)
    print("%d checks, %d failed" % (len(results), results.count(False)))
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
