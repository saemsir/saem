#!/usr/bin/env python3
"""Generate the RISC-V instruction tables of the sovereign C17 assembly layer from riscv-opcodes.

    python3 tools/gen_rv_tab.py                 write core/c17/cos_rv_ops.h and core/c17/cos_rv_tab.c
    python3 tools/gen_rv_tab.py --check         regenerate in memory and fail if the committed files differ
    python3 tools/gen_rv_tab.py --opcodes DIR   another checkout of https://github.com/riscv/riscv-opcodes

Why: the C17 code must contain NO hand-typed opcode.  The machine-readable files of riscv-opcodes say, for every instruction, which bits are fixed
(opcode, funct3, funct7, ...) and which operand fields exist.  This tool turns them into (match, mask, class) rows; the C code only knows the
operand FORMATS (R, I, S, B, U, J, ...), which are tiny and are proven against LLVM (tools/check_rv.py).
Self-checks done here, at generation time (the tool refuses to write if one fails):
  - every operand name is in arg_lut.csv; every instruction covers all 32 bits exactly once (operand fields + fixed bits, no overlap, no gap);
  - the operand set of every instruction is a known class (a new, unknown shape stops the tool instead of producing a wrong encoder);
  - no two instructions that exist in the same register width have overlapping (match, mask): decoding is unambiguous.
Python standard library only (this is the frontend's "metadata orchestration" role: the output is plain C data).
"""
import argparse
import csv
import hashlib
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "core", "c17")
DEFAULT_OPCODES = "/home/claude/riscv/riscv-opcodes"

# (file, extension tag, register widths it exists in).  Extension tag "" = base integer ISA (always on).
SETS = [
    ("rv_i", "", (32, 64)),
    ("rv32_i", "", (32,)),      # only the three shift redefinitions (slli, srli, srai with a 5-bit shift amount) are taken, see PSEUDO_REAL
    ("rv64_i", "", (64,)),
    ("rv_m", "M", (32, 64)),
    ("rv64_m", "M", (64,)),
    ("rv_a", "A", (32, 64)),
    ("rv64_a", "A", (64,)),
    ("rv_zicsr", "ZICSR", (32, 64)),
    ("rv_zifencei", "ZIFENCEI", (32, 64)),
    ("rv_system", "SYSTEM", (32, 64)),   # mret, wfi (machine mode)
]
PSEUDO_REAL = {"rv32_i": {"slli", "srli", "srai"}}   # `$pseudo_op rv64_i::slli slli ...` in rv32_i REDEFINES the real shift for RV32

# operand-name set -> class.  The load and jalr shapes are the I shape; they are told apart by the major opcode below.
SHAPE_LIST = [
    (["rd", "rs1", "rs2"], "R"),
    (["rd", "rs1", "imm12"], "I"),
    (["rd", "rs1", "shamtw"], "SH5"),
    (["rd", "rs1", "shamtd"], "SH6"),
    (["imm12hi", "rs1", "rs2", "imm12lo"], "S"),
    (["bimm12hi", "rs1", "rs2", "bimm12lo"], "B"),
    (["rd", "imm20"], "U"),
    (["rd", "jimm20"], "J"),
    (["rd", "rs1", "rs2", "aq", "rl"], "AMO"),
    (["rd", "rs1", "aq", "rl"], "LR"),
    (["rd", "rs1", "csr"], "CSR"),
    (["rd", "csr", "zimm5"], "CSRI"),
    (["fm", "pred", "succ", "rs1", "rd"], "FENCE"),
    ([], "N"),
]
SHAPES = {}
for _ops, _cls in SHAPE_LIST:
    assert frozenset(_ops) not in SHAPES, "two classes share the operand set %s" % sorted(_ops)  # a dict literal would silently keep the last one
    SHAPES[frozenset(_ops)] = _cls
NAME_CLASS = {"fence.i": "FENCEI"}   # same operand set as the I shape (imm12 rs1 rd), different meaning: told apart by name
CLASSES = ["R", "I", "L", "JR", "SH5", "SH6", "S", "B", "U", "J", "AMO", "LR", "CSR", "CSRI", "FENCE", "FENCEI", "N"]
EXT_BIT = {"": 0, "M": 1, "A": 2, "ZICSR": 4, "ZIFENCEI": 8, "SYSTEM": 16}
CSR_KEEP = re.compile(r"^(m(status|statush|isa|edeleg|ideleg|ie|tvec|counteren|envcfg|countinhibit|scratch|epc|cause|tval|ip|seccfg|cycle|cycleh|instret|instreth"
                      r"|vendorid|archid|impid|hartid|configptr)|pmp(cfg|addr)\d+|cycle|time|instret|cycleh|timeh|instreth)$")
FIXED = re.compile(r"^(\d+)(?:\.\.(\d+))?=(0x[0-9a-fA-F]+|\d+)$")


def read_lut(d):
    lut = {}
    with open(os.path.join(d, "arg_lut.csv"), newline="") as f:
        for row in csv.reader(f, skipinitialspace=True):
            if len(row) >= 3:
                lut[row[0].strip().strip('"')] = (int(row[1]), int(row[2]))
    return lut


def parse_file(d, name, lut):
    """Yield (mnemonic, [operand names], match, mask) for each real instruction of one extension file."""
    out = []
    with open(os.path.join(d, "extensions", name)) as f:
        for ln, raw in enumerate(f, 1):
            line = raw.split("#")[0].strip()
            if not line:
                continue
            toks = line.split()
            if toks[0] == "$pseudo_op":
                if name not in PSEUDO_REAL or toks[2] not in PSEUDO_REAL[name]:
                    continue
                toks = toks[2:]            # name args... fixed...
            elif toks[0].startswith("$"):
                raise SystemExit("%s:%d: unknown directive %s" % (name, ln, toks[0]))
            mnem, rest = toks[0], toks[1:]
            ops, match, mask, covered = [], 0, 0, 0
            for t in rest:
                m = FIXED.match(t)
                if m:
                    hi = int(m.group(1))
                    lo = int(m.group(2)) if m.group(2) is not None else hi
                    val = int(m.group(3), 0)
                    if hi < lo or hi > 31 or val >> (hi - lo + 1):
                        raise SystemExit("%s:%d: bad fixed field %s" % (name, ln, t))
                    bits = ((1 << (hi - lo + 1)) - 1) << lo
                    if covered & bits:
                        raise SystemExit("%s:%d: %s overlaps another field" % (name, ln, t))
                    covered |= bits
                    mask |= bits
                    match |= val << lo
                elif t in lut:
                    hi, lo = lut[t]
                    bits = ((1 << (hi - lo + 1)) - 1) << lo
                    if covered & bits:
                        raise SystemExit("%s:%d: operand %s overlaps another field" % (name, ln, t))
                    covered |= bits
                    ops.append(t)
                else:
                    raise SystemExit("%s:%d: unknown token %s (not in arg_lut.csv)" % (name, ln, t))
            if covered != 0xFFFFFFFF:
                raise SystemExit("%s:%d: %s leaves bits uncovered: %08x" % (name, ln, mnem, covered ^ 0xFFFFFFFF))
            out.append((mnem, ops, match, mask))
    return out


def build(d):
    lut = read_lut(d)
    rows = []
    for fname, ext, widths in SETS:
        for mnem, ops, match, mask in parse_file(d, fname, lut):
            shape = frozenset(ops)
            if shape not in SHAPES:
                raise SystemExit("%s: %s has an operand set no class knows: %s" % (fname, mnem, sorted(ops)))
            cls = NAME_CLASS.get(mnem) or SHAPES[shape]
            if cls == "I":
                cls = {0x03: "L", 0x67: "JR"}.get(match & 0x7F, "I")
            xl = (1 if 32 in widths else 0) | (2 if 64 in widths else 0)
            rows.append(dict(name=mnem, match=match, mask=mask, cls=cls, xlens=xl, ext=EXT_BIT[ext], src=fname))
    # unambiguous decoding inside each register width
    for i, a in enumerate(rows):
        for b in rows[i + 1:]:
            if a["xlens"] & b["xlens"] and (a["mask"] & b["mask"] & (a["match"] ^ b["match"])) == 0:
                raise SystemExit("ambiguous decoding: %s (%s) and %s (%s)" % (a["name"], a["src"], b["name"], b["src"]))
    names = []
    for r in rows:
        if r["name"] not in names:
            names.append(r["name"])
        r["id"] = names.index(r["name"])
    csrs = []  # (address, widths, name); widths: bit 0 = exists in RV32, bit 1 = exists in RV64
    for fn, widths in (("csrs.csv", 3), ("csrs32.csv", 1)):  # csrs32.csv lists the registers that exist only in RV32 (the *h upper halves)
        with open(os.path.join(d, fn), newline="") as f:
            for row in csv.reader(f, skipinitialspace=True):
                if len(row) >= 2:
                    nm = row[1].strip().strip('"')
                    if CSR_KEEP.match(nm):
                        w = widths
                        m = re.fullmatch(r"pmpcfg(\d+)", nm)
                        if m and int(m.group(1)) % 2 == 1:
                            w = 1  # riscv-privileged 3.7.1: for RV64 "the odd-numbered configuration registers, pmpcfg1, pmpcfg3, ..., pmpcfg15, are illegal"
                        csrs.append((int(row[0], 0), w, nm))
    csrs.sort()
    seen = set()
    for a, _, n in csrs:
        if a in seen or n in seen:
            raise SystemExit("duplicate CSR %s %x" % (n, a))
        seen.add(a), seen.add(n)
    return rows, names, csrs


def ident(n):
    return re.sub(r"[^A-Za-z0-9]", "_", n).upper()


def sources_stamp(d):
    h = hashlib.sha256()
    for fname, _, _ in SETS:
        h.update(open(os.path.join(d, "extensions", fname), "rb").read())
    for fn in ("arg_lut.csv", "csrs.csv", "csrs32.csv"):
        h.update(open(os.path.join(d, fn), "rb").read())
    try:
        rev = subprocess.run(["git", "-C", d, "rev-parse", "HEAD"], capture_output=True, text=True, timeout=20).stdout.strip() or "unknown"
    except Exception:
        rev = "unknown"
    return rev, h.hexdigest()


def emit(d):
    rows, names, csrs = build(d)
    rev, digest = sources_stamp(d)
    head = ("/* GENERATED by tools/gen_rv_tab.py from riscv-opcodes (https://github.com/riscv/riscv-opcodes) revision %s\n"
            " * input digest (sha256 of the extension files, arg_lut.csv, csrs.csv, csrs32.csv): %s\n"
            " * DO NOT EDIT.  `python3 tools/gen_rv_tab.py --check` fails if this file is not what the generator produces. */\n" % (rev, digest))
    h = [head, "#ifndef COS_RV_OPS_H\n#define COS_RV_OPS_H\n\n",
         "/* Extension bits (the base integer ISA is always on). */\n",
         "#define COS_RV_EXT_M 1u\n#define COS_RV_EXT_A 2u\n#define COS_RV_EXT_ZICSR 4u\n#define COS_RV_EXT_ZIFENCEI 8u\n#define COS_RV_EXT_SYSTEM 16u\n",
         "#define COS_RV_EXT_ALL 31u\n\n",
         "/* Operand formats.  Assembly syntax and field layout depend only on this. */\ntypedef enum CosRvCls {\n"]
    h += ["  COS_RV_CLS_%s,\n" % c for c in CLASSES]
    h += ["  COS_RV_CLS_COUNT\n} CosRvCls;\n\n",
          "/* One enumerator per mnemonic.  An instruction that exists in both widths with different encodings (slli, srli, srai) has one id and two rows. */\n",
          "typedef enum CosRvOp {\n"]
    h += ["  COS_RV_%s,\n" % ident(n) for n in names]
    h += ["  COS_RV_OP_COUNT\n} CosRvOp;\n\n",
          "#define COS_RV_ROWS %du\n#define COS_RV_CSRS %du\n\n#endif\n" % (len(rows), len(csrs))]
    c = [head, '#include "cos_rv.h"\n\n',
         "const CosRvRow cos_rv_rows[COS_RV_ROWS] = {\n"]
    for r in rows:
        c.append('  {0x%08Xu, 0x%08Xu, "%s", COS_RV_%s, COS_RV_CLS_%s, %d, %d}, /* %s */\n' % (
            r["match"], r["mask"], r["name"], ident(r["name"]), r["cls"], r["xlens"], r["ext"], r["src"]))
    c.append("};\n\nconst CosRvCsrName cos_rv_csr_names[COS_RV_CSRS] = {\n")
    for a, w, n in csrs:
        c.append('  {0x%03Xu, %d, "%s"},\n' % (a, w, n))
    c.append("};\n")
    return "".join(h), "".join(c), len(rows), len(names), len(csrs)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--opcodes", default=DEFAULT_OPCODES)
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--out-dir", default=OUT)
    a = ap.parse_args()
    if not os.path.isdir(os.path.join(a.opcodes, "extensions")):
        print("riscv-opcodes not found at %s (clone https://github.com/riscv/riscv-opcodes; the committed tables stay valid)" % a.opcodes, file=sys.stderr)
        return 2
    hdr, src, nrows, nnames, ncsr = emit(a.opcodes)
    ph, ps = os.path.join(a.out_dir, "cos_rv_ops.h"), os.path.join(a.out_dir, "cos_rv_tab.c")
    if a.check:
        ok = os.path.exists(ph) and os.path.exists(ps) and open(ph).read() == hdr and open(ps).read() == src
        print("gen_rv_tab --check: %s (%d rows, %d mnemonics, %d CSR names)" % ("tables match riscv-opcodes" if ok else "TABLES DIFFER", nrows, nnames, ncsr))
        return 0 if ok else 1
    with open(ph, "w", newline="\n") as f:
        f.write(hdr)
    with open(ps, "w", newline="\n") as f:
        f.write(src)
    print("wrote %s and %s: %d rows, %d mnemonics, %d CSR names" % (ph, ps, nrows, nnames, ncsr))
    return 0


if __name__ == "__main__":
    sys.exit(main())
