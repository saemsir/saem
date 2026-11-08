#!/usr/bin/env python3
"""One command that checks the whole C17 sovereign kernel (core/c17/).

    python3 tools/check_c17.py            everything (about a minute on two cores)
    python3 tools/check_c17.py --quick    compile, structure checks, cross-checks, and the tests at -O2 only

What it proves, so the design rules are checks and not promises:
  compile     every source builds with -Werror under gcc (strict + extra warnings, incl. no VLA/alloca, stack use per function <= 2 KB) and clang,
              and with -mgeneral-regs-only (the compiler itself refuses floating point and vector registers)
  headers     every header stands alone in C17 and also parses as C++17 (extern "C" boundary)
  zero heap   the objects import nothing but memcpy/memset/memcmp/memmove/strlen (and the compiler's stack check); no malloc, free, stdio, math;
              the sources contain no malloc/free/alloca and include only freestanding headers
  zero float  no float/double/math in the sources (comments and strings stripped)
  no globals  no writable data symbols and no .data/.bss in any object: all state lives in memory the caller hands in
  atomics     only core/c17/cos_atomic.h touches a hardware atomic (no direct atomic_*, no <stdatomic.h> elsewhere in the kernel); it has two
              profiles, many harts (default) and -DCOS_SINGLE_HART=1 (the monitor core: relaxed load/store only), and every source builds in both
  riscv       the kernel built for RV64IMAC (the monitor core's profile) by clang/lld: no float, no privileged instruction, inline atomics, links;
              every lr/sc loop obeys the manual's constrained-loop rules; the one-hart profile has no lr/sc/amo/fence at all
  tests       all test programs at -O0, -O2, -O3, gcc+clang, under AddressSanitizer+UBSan, and the threaded ones under ThreadSanitizer;
              the programs that use atomics also run as the one-hart profile (no threads there: a re-entry test instead)
  cross-checks C SHA-256/HMAC against Python's hashlib/hmac on thousands of inputs; against the C++ engine's SHA-256; the constitution seal
              digest against tools/seal.py (inside the core test); the Zone 0 gate against Python's Unicode database for every code point;
              the exact-rational layer (add, sub, mul, div, pow, cmp, parse, text) against Python's arbitrary-precision Fraction on 150,000+ cases
Exit code 0 only if everything passes.
"""
import argparse
import concurrent.futures as cf
import glob
import hashlib
import hmac
import os
import random
import re
import subprocess
import sys
import unicodedata

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C17 = os.path.join(ROOT, "core", "c17")
TESTS = os.path.join(ROOT, "core", "tests")
BUILD = os.path.join(ROOT, "build_c17")
OBJ = os.path.join(BUILD, "obj")
sys.path.insert(0, os.path.join(ROOT, "tools"))

STRICT = ["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
EXTRA = ["-Wvla", "-Walloca", "-Wundef", "-Wcast-align", "-Wpointer-arith", "-Wdouble-promotion", "-Wfloat-equal", "-Wconversion", "-Wshadow",
         "-Wstrict-prototypes", "-Wmissing-prototypes", "-Wcast-qual", "-Wformat=2", "-Wnull-dereference", "-Wduplicated-cond", "-Wlogical-op",
         "-Wstack-usage=2048"]
INC = ["-I" + C17, "-I" + TESTS, "-I" + BUILD]
ALLOWED_UNDEFINED = {"memcpy", "memmove", "memset", "memcmp", "strlen", "__stack_chk_fail"}
ALLOWED_SYSTEM_HEADERS = {"stdint.h", "stddef.h", "stdbool.h", "stdalign.h", "stdatomic.h", "string.h"}
C_ONLY_HEADERS = {"cos_atomic.h"}  # C17 only by design (_Atomic, _Generic): C++ meets the kernel through the extern "C" door, not through this header
FORBIDDEN_CALLS = ["malloc", "calloc", "realloc", "free", "aligned_alloc", "posix_memalign", "memalign", "valloc", "strdup", "alloca", "mmap", "sbrk", "brk"]

# test program -> (test source, kernel sources it needs, uses threads)
PROGRAMS = {
    "sha256": ("c17_sha256_test.c", ["cos_sha256.c"], False),
    "pool": ("c17_pool_test.c", ["cos_pool.c"], True),
    "regs": ("c17_regs_test.c", ["cos_regs.c"], False),
    "wal": ("c17_wal_test.c", ["cos_wal.c", "cos_sha256.c"], False),
    "journal": ("c17_journal_test.c", ["cos_journal.c", "cos_wal.c", "cos_sha256.c"], True),
    "rat": ("c17_rat_test.c", ["cos_rat.c"], False),
    "alg": ("c17_alg_test.c", ["cos_alg.c", "cos_rat.c"], False),
    "mat": ("c17_mat_test.c", ["cos_mat.c", "cos_rat.c"], False),
    "gate": ("c17_gate_test.c", ["cos_gate.c"], True),
    "core": ("c17_core_test.c", ["cos_core.c", "cos_gate.c", "cos_seal.c", "cos_journal.c", "cos_wal.c", "cos_sha256.c"], True),
    "rv": ("c17_rv_test.c", ["cos_rv.c", "cos_rv_tab.c"], False),
}

results = []  # (ok, title, detail)


def record(ok, title, detail=""):
    results.append((ok, title, detail))
    print("%s  %s" % ("ok  " if ok else "FAIL", title), flush=True)
    if detail and not ok:
        for line in detail.strip().splitlines()[:40]:
            print("      " + line, flush=True)


def run(cmd, **kw):
    kw.setdefault("capture_output", True)
    kw.setdefault("text", True)
    kw.setdefault("timeout", 900)
    return subprocess.run(cmd, **kw)


def kernel_sources():
    return sorted(glob.glob(os.path.join(C17, "*.c")))


def kernel_headers():
    return sorted(glob.glob(os.path.join(C17, "*.h")))


# ---------------------------------------------------------------- source text rules
def strip_c(src):
    """Comments removed, string and character literal contents blanked; newlines kept so line numbers stay right."""
    out, i, n = [], 0, len(src)
    while i < n:
        if src.startswith("/*", i):
            j = src.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * src.count("\n", i, j) or " ")
            i = j
        elif src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif src[i] in "\"'":
            q = src[i]
            j = i + 1
            while j < n and src[j] != q:
                j += 2 if src[j] == "\\" else 1
            out.append(q + q)
            i = j + 1
        else:
            out.append(src[i])
            i += 1
    return "".join(out)


FLOAT_WORDS = re.compile(r"\b(float|double|_Complex|_Imaginary)\b")
FLOAT_LITERAL = re.compile(r"(?<![\w.])(\d+\.\d*|\.\d+|\d+[eE][+-]?\d+)(?![\w.])")
HEX_LITERAL = re.compile(r"\b0[xX][0-9a-fA-F']+[uUlL]*\b")
CALL_WORDS = re.compile(r"\b(" + "|".join(FORBIDDEN_CALLS) + r")\b")
INCLUDE = re.compile(r"^\s*#\s*include\s*<([^>]+)>", re.M)
ATOMIC_USE = re.compile(r"\batomic_(load|store|exchange|fetch_\w+|compare_exchange_\w+|flag\w*|init|thread_fence|signal_fence|is_lock_free)\b|\b__atomic_\w+|\b__sync_\w+")


def check_source_rules():
    bad = []
    files = kernel_sources() + kernel_headers()
    for path in files:
        code = strip_c(open(path, encoding="utf-8").read())
        name = os.path.relpath(path, ROOT)
        for m in INCLUDE.finditer(open(path, encoding="utf-8").read()):
            if m.group(1) not in ALLOWED_SYSTEM_HEADERS:
                bad.append("%s: includes <%s>; the kernel may use only %s" % (name, m.group(1), ", ".join(sorted(ALLOWED_SYSTEM_HEADERS))))
            if m.group(1) == "stdatomic.h" and os.path.basename(path) != "cos_atomic.h":
                bad.append("%s: includes <stdatomic.h>; only cos_atomic.h may (the profile switch lives there)" % name)
        nohex = HEX_LITERAL.sub("0", code)
        for lineno, line in enumerate(nohex.splitlines(), 1):
            if os.path.basename(path) != "cos_atomic.h" and ATOMIC_USE.search(line):
                bad.append("%s:%d: direct atomic call, use the cos_* wrappers of cos_atomic.h: %s" % (name, lineno, line.strip()))
            if FLOAT_WORDS.search(line) or FLOAT_LITERAL.search(line):
                bad.append("%s:%d: floating point: %s" % (name, lineno, line.strip()))
            m = CALL_WORDS.search(line)
            if m:
                bad.append("%s:%d: forbidden call %s: %s" % (name, lineno, m.group(1), line.strip()))
    record(not bad, "sources: %d files, no float/double, no malloc/free/alloca, only freestanding includes, atomics only through cos_atomic.h" % len(files), "\n".join(bad))


# ---------------------------------------------------------------- compile and structure
def compile_one(src, extra, cc="gcc", out=None):
    cmd = [cc] + STRICT + ["-O2"] + extra + INC + ["-c", src, "-o", out or os.devnull]
    p = run(cmd)
    return p.returncode == 0, (p.stderr or "")[:1500]


def check_compile():
    os.makedirs(OBJ, exist_ok=True)
    srcs = kernel_sources()
    jobs = []
    with cf.ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as ex:
        for s in srcs:
            b = os.path.basename(s)[:-2]
            jobs.append(("gcc strict", s, ex.submit(compile_one, s, [], "gcc", os.path.join(OBJ, b + ".o"))))
            jobs.append(("gcc extra", s, ex.submit(compile_one, s, EXTRA, "gcc")))
            jobs.append(("clang", s, ex.submit(compile_one, s, ["-Wconversion", "-Wshadow"], "clang")))
            jobs.append(("general-regs-only", s, ex.submit(compile_one, s, ["-mgeneral-regs-only"], "gcc")))
            jobs.append(("gcc one-hart", s, ex.submit(compile_one, s, EXTRA + ["-DCOS_SINGLE_HART=1"], "gcc")))
            jobs.append(("clang one-hart", s, ex.submit(compile_one, s, ["-Wconversion", "-Wshadow", "-DCOS_SINGLE_HART=1"], "clang")))
    errors = []
    for kind, s, fut in jobs:
        ok, err = fut.result()
        if not ok:
            errors.append("[%s] %s\n%s" % (kind, os.path.basename(s), err))
    record(not errors, "compile: %d sources x (gcc strict, gcc +extra warnings and 2 KB stack limit, clang, -mgeneral-regs-only, and the one-hart profile under gcc and clang)" % len(srcs), "\n".join(errors))


def check_headers():
    errors = []
    for h in kernel_headers():
        name = os.path.basename(h)
        p = run(["gcc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-fsyntax-only", "-x", "c", "-", "-I" + C17], input='#include "%s"\n' % name)
        if p.returncode != 0:
            errors.append("C17 alone: %s\n%s" % (name, p.stderr[:600]))
        p = run(["gcc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-fsyntax-only", "-DCOS_SINGLE_HART=1", "-x", "c", "-", "-I" + C17], input='#include "%s"\n' % name)
        if p.returncode != 0:
            errors.append("C17 alone, one-hart profile: %s\n%s" % (name, p.stderr[:600]))
    cpp = [h for h in kernel_headers() if os.path.basename(h) not in C_ONLY_HEADERS]
    tu = "".join('#include "%s"\n' % os.path.basename(h) for h in cpp) + "int main(void) { return 0; }\n"
    p = run(["g++", "-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-fsyntax-only", "-x", "c++", "-", "-I" + C17], input=tu)
    if p.returncode != 0:
        errors.append("C++17 together\n" + p.stderr[:800])
    record(not errors, "headers: %d stand alone in C17 (both atomic profiles) and %d parse together as C++17 (C17-only by design: %s)" % (
        len(kernel_headers()), len(cpp), ", ".join(sorted(C_ONLY_HEADERS))), "\n".join(errors))


def nm(args, obj):
    p = run(["nm"] + args + [obj])
    return [ln.split() for ln in p.stdout.splitlines() if ln.strip()]


def check_objects():
    objs = sorted(glob.glob(os.path.join(OBJ, "*.o")))
    defined, undefined, writable, sections = set(), {}, [], []
    for o in objs:
        base = os.path.basename(o)
        for parts in nm(["--defined-only"], o):
            if len(parts) == 3:
                _, t, name = parts
                defined.add(name)
                if t in "DdBbCcGgSsVvWw":
                    writable.append("%s: writable symbol %s (type %s)" % (base, name, t))
        for parts in nm(["-u"], o):
            undefined.setdefault(parts[-1], []).append(base)
        sz = run(["size", "-A", o]).stdout
        for ln in sz.splitlines():
            f = ln.split()
            if len(f) >= 2 and f[1].isdigit() and int(f[1]) > 0 and re.match(r"^\.(data|bss|tbss|tdata|init_array|fini_array)(\.|$)", f[0]) \
                    and not f[0].startswith(".data.rel.ro"):
                sections.append("%s: section %s has %s bytes" % (base, f[0], f[1]))
    heap = []
    for name, users in sorted(undefined.items()):
        if name in defined or name in ALLOWED_UNDEFINED:
            continue
        heap.append("%s imports %s" % (", ".join(users), name))
    record(not heap, "zero heap: %d objects import only memcpy/memmove/memset/memcmp/strlen and each other (no malloc, free, stdio, math)" % len(objs),
           "\n".join(heap))
    record(not writable and not sections, "no global state: no writable data symbol and no .data/.bss section in any kernel object",
           "\n".join(writable + sections))


# ---------------------------------------------------------------- tests
def build_and_run(name, variant):
    test, srcs, threads = PROGRAMS[name]
    flags = {"O0": ["-O0"], "O2": ["-O2"], "O3": ["-O3"], "clang-O2": ["-O2"],
             "asan": ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"],
             "tsan": ["-O1", "-g", "-fsanitize=thread"],
             # the monitor-core profile (cos_atomic.h): relaxed load/store only; the test programs skip their threads and run the re-entry test instead
             "sh-O2": ["-O2", "-DCOS_SINGLE_HART=1"], "sh-clang-O2": ["-O2", "-DCOS_SINGLE_HART=1"],
             "sh-asan": ["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-DCOS_SINGLE_HART=1"]}[variant]
    cc = "clang" if variant.endswith("clang-O2") else "gcc"
    exe = os.path.join(BUILD, "t_%s_%s" % (name, variant))
    cmd = [cc] + STRICT + flags + (["-pthread"] if threads else []) + INC + [os.path.join(TESTS, test)] + [os.path.join(C17, s) for s in srcs] + ["-o", exe]
    p = run(cmd)
    if p.returncode != 0:
        return False, "build failed\n" + p.stderr[:1500]
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=1:abort_on_error=0", TSAN_OPTIONS="halt_on_error=1")
    q = run([exe], env=env)
    tail = (q.stdout + q.stderr).strip().splitlines()
    last = tail[-1] if tail else ""
    ok = q.returncode == 0 and re.search(r"\b0 failed\b", last) is not None
    return ok, "\n".join(tail[-12:])


def seal_vector():
    """Generate the C text of the seal test vector from the JSON spec with the independent Python tool."""
    import seal
    os.makedirs(BUILD, exist_ok=True)
    spec = seal.load_spec(os.path.join(TESTS, "vectors", "seal_vector.json"))
    key = "QCOS-TEST-KEY-NOT-FOR-RELEASE-0001".encode()
    text = seal.emit_c(spec, "TV", key, "core/tests/vectors/seal_vector.json")
    with open(os.path.join(BUILD, "seal_vector.inc"), "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def check_tests(quick):
    variants = ["O2"] if quick else ["O0", "O2", "O3", "clang-O2", "asan", "tsan"]
    one_hart = ["sh-O2"] if quick else ["sh-O2", "sh-clang-O2", "sh-asan"]
    jobs = []
    with cf.ThreadPoolExecutor(max_workers=os.cpu_count() or 2) as ex:
        for name, (_, srcs, threads) in PROGRAMS.items():
            uses_atomics = any("cos_atomic.h" in open(os.path.join(C17, f), encoding="utf-8").read() for f in srcs)
            for v in variants + (one_hart if uses_atomics else []):
                if v == "tsan" and not threads:
                    continue
                jobs.append((name, v, ex.submit(build_and_run, name, v)))
    table = {}
    for name, v, fut in jobs:
        ok, detail = fut.result()
        table.setdefault(name, []).append((v, ok, detail))
    for name in PROGRAMS:
        rows = table.get(name, [])
        ok = all(r[1] for r in rows)
        record(ok, "test %-8s %s" % (name, " ".join("%s%s" % (v, "" if o else "(FAIL)") for v, o, _ in rows)),
               "\n".join("[%s]\n%s" % (v, d) for v, o, d in rows if not o))


# ---------------------------------------------------------------- cross-checks
def build_dump():
    exe = os.path.join(BUILD, "c17_dump")
    p = run(["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Werror"] + INC +
            [os.path.join(TESTS, "c17_dump.c"), os.path.join(C17, "cos_gate.c"), os.path.join(C17, "cos_sha256.c"), os.path.join(C17, "cos_rat.c"), "-o", exe])
    if p.returncode != 0:
        raise RuntimeError(p.stderr)
    return exe


def check_hash(dump):
    rng = random.Random(20261030)
    lines, want = [], []
    sizes = list(range(0, 140)) + [191, 192, 255, 256, 257, 1000, 4096, 10000]
    keys = [None, b"k", bytes(range(32)), bytes(range(64)), bytes(range(65)), bytes(range(131)), bytes(200)]
    for n in sizes:
        msg = bytes(rng.getrandbits(8) for _ in range(n))
        for key in keys[: 1 + (n % 7)]:
            lines.append("%s %s" % ("-" if key is None else key.hex(), msg.hex()))
            want.append((hashlib.sha256(msg).hexdigest(), "-" if key is None else hmac.new(key, msg, hashlib.sha256).hexdigest()))
    p = run([dump, "hash"], input="\n".join(lines) + "\n")
    got = [tuple(ln.split()) for ln in p.stdout.splitlines()]
    bad = [i for i, (g, w) in enumerate(zip(got, want)) if g != w]
    ok = p.returncode == 0 and len(got) == len(want) and not bad
    record(ok, "cross-check: C SHA-256 and HMAC-SHA-256 equal Python hashlib/hmac on %d inputs (lengths 0..10000, keys 0..200 bytes)" % len(want),
           "returncode %s, %d of %d lines, first mismatch at %s" % (p.returncode, len(got), len(want), bad[:3]))


def check_cpp_sha():
    exe = os.path.join(BUILD, "sha_vs_cpp")
    o = os.path.join(BUILD, "cos_sha256_for_cpp.o")  # not in obj/: that directory holds exactly the kernel objects that get inspected
    p = run(["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Werror", "-I" + C17, "-c", os.path.join(C17, "cos_sha256.c"), "-o", o])
    if p.returncode == 0:
        p = run(["g++", "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror", "-I" + C17, os.path.join(TESTS, "c17_sha256_vs_cpp.cpp"), o, "-o", exe])
    if p.returncode != 0:
        record(False, "cross-check: C SHA-256 vs the C++ engine's SHA-256", "build failed\n" + p.stderr[:1200])
        return
    q = run([exe])
    record(q.returncode == 0, "cross-check: C SHA-256 equals the C++ engine's SHA-256 (all lengths 0..700, split and bursty updates)", q.stdout + q.stderr)


SCRIPT_PREFIXES = ("DEVANAGARI", "COMBINING DEVANAGARI", "BENGALI", "GURMUKHI", "GUJARATI", "ORIYA", "TAMIL", "TELUGU", "KANNADA", "MALAYALAM", "SINHALA", "ARABIC")
RULE = dict(none=0, length=1, utf8=2, nul=3, control=4, bidi=5, invisible=6, joiner=7, private=8, nonchar=9)


def check_unicode(dump):
    p = run([dump, "gate"])
    parts = p.stdout.split("\n")
    n = 0x110000
    if p.returncode != 0 or len(parts) < 3 or len(parts[0]) != n or len(parts[1]) != n or len(parts[2]) != n:
        record(False, "cross-check: Zone 0 gate against the Unicode database", "dump failed: %s lengths %s" % (p.returncode, [len(x) for x in parts]))
        return
    alone = [int(c, 32) for c in parts[0]]
    after_zwj = [int(c, 32) for c in parts[1]]
    after_zwnj = [int(c, 32) for c in parts[2]]
    errors = []

    def want(cp, rule, why):
        if alone[cp] != rule and len(errors) < 40:
            errors.append("U+%04X (%s): gate says %d, expected %d" % (cp, why, alone[cp], rule))

    bidi = set(range(0x202A, 0x202F)) | set(range(0x2066, 0x206A))
    invisible = {0x200B, 0xFEFF, 0x180E} | set(range(0x2060, 0x2065)) | set(range(0xE0000, 0xE0080)) | set(range(0xE0100, 0xE01F0))
    accepted_cf, unassigned_accepted, cats = [], 0, {}
    for cp in range(n):
        c = unicodedata.category(chr(cp))
        if cp in bidi:
            want(cp, RULE["bidi"], "bidi control")
        elif cp in invisible:
            want(cp, RULE["invisible"], "invisible")
        elif cp in (0x200C, 0x200D):
            want(cp, RULE["joiner"], "joiner with nothing before it")
        elif c == "Cs":
            want(cp, RULE["utf8"], "surrogate")
        elif c == "Cc":
            want(cp, RULE["none"] if cp in (9, 10, 13) else (RULE["nul"] if cp == 0 else RULE["control"]), "control")
        elif 0xFDD0 <= cp <= 0xFDEF or (cp & 0xFFFE) == 0xFFFE or cp == 0xFFFD or 0xFFF9 <= cp <= 0xFFFC:
            want(cp, RULE["nonchar"], "noncharacter or damaged-text marker")
        elif c == "Co":
            want(cp, RULE["private"], "private use")
        else:
            want(cp, RULE["none"], "category " + c)
            if c == "Cf":
                accepted_cf.append(cp)
            if c == "Cn":
                unassigned_accepted += 1
        cats[c] = cats.get(c, 0) + 1
    record(not errors, "cross-check: Zone 0 gate agrees with Python's Unicode database (Unicode %s) on all 1,114,112 code points" % unicodedata.unidata_version,
           "\n".join(errors))

    # Hindi, Bengali, Tamil ... Urdu and Persian must never lose their joiners: every assigned BMP letter, mark, digit or sign of these scripts
    # accepts a ZWJ and a ZWNJ right after it
    missed = []
    covered = 0
    for cp in range(0x10000):
        try:
            name = unicodedata.name(chr(cp))
        except ValueError:
            continue
        if name.startswith(SCRIPT_PREFIXES) and alone[cp] == RULE["none"]:
            covered += 1
            if after_zwj[cp] != 0 or after_zwnj[cp] != 0:
                missed.append("U+%04X %s" % (cp, name))
    record(not missed and covered > 1500, "cross-check: ZWJ and ZWNJ are accepted after all %d assigned BMP characters of the Indian and Arabic scripts" % covered,
           "%d refused, e.g. %s" % (len(missed), "; ".join(missed[:8])))
    # a joiner after anything else is refused (checked on a sample of assigned characters of other scripts)
    leaked = [cp for cp in range(0x10000) if alone[cp] == 0 and unicodedata.category(chr(cp)) in ("Lu", "Ll", "Nd", "Zs", "Sm", "Sc", "Po")
              and not (0x0600 <= cp <= 0x06FF or 0x0750 <= cp <= 0x077F or 0x0870 <= cp <= 0x08FF or 0x0900 <= cp <= 0x0DFF or 0xA8E0 <= cp <= 0xA8FF
                       or 0xFB50 <= cp <= 0xFDFF or 0xFE70 <= cp <= 0xFEFE) and after_zwj[cp] == 0]
    record(not leaked, "cross-check: a joiner after Latin, Greek, Cyrillic, CJK, digits, spaces and symbols is refused", "%d accepted, e.g. %s" % (len(leaked), leaked[:8]))
    print("      info: %d format characters (Cf) are accepted on purpose (soft hyphen, Arabic number signs, LRM/RLM, ...); %d unassigned code points are "
          "accepted because the gate carries no Unicode table" % (len(accepted_cf), unassigned_accepted), flush=True)


INT64_MAX = 2**63 - 1
INT64_MIN = -2**63
NUM_RE = re.compile(rb"(-?)(0|[1-9][0-9]*)(?:\.([0-9]+))?(?:[eE]([+-]?[0-9]+))?")


def check_rat(dump):
    """cos_rat.c against Python's arbitrary-precision integers and fractions. For every operation the C answer must be the EXACT answer or a refusal;
    add, sub, mul, div and cmp are decided exactly; pow follows square-and-multiply; the number reader follows the JSON grammar; the text writers are compared digit by digit."""
    from fractions import Fraction
    from math import gcd
    rng = random.Random(20261031)
    M = INT64_MAX
    cmds, want, kinds = [], [], []

    def res(st, fr=None):
        return "%d %d %d" % (st, fr.numerator, fr.denominator) if st == 0 else "%d 12345 678" % st

    def fits(fr):
        return abs(fr.numerator) <= M and fr.denominator <= M

    def exp_addsub(a, b, sub):
        if sub:
            b = -b
        exact = a + b
        return (res(0, exact), "ok") if fits(exact) else (res(15), "refused")

    def exp_mul(a, b):
        r = a * b
        return (res(0, r), "ok") if fits(r) else (res(15), "refused")

    def exp_div(a, b):
        if b == 0:
            return res(8), "invalid"
        return exp_mul(a, 1 / b)

    def exp_pow(a, e):
        if a == 0 and e <= 0:
            return res(8), "invalid"
        base, k = a, e
        if k < 0:
            k, base = -k, 1 / a
        acc = Fraction(1)
        while k > 0:
            if k & 1:
                acc = acc * base
                if not fits(acc):
                    return res(15), "refused"
            k >>= 1
            if k > 0:
                base = base * base
                if not fits(base):
                    return res(15), "refused"
        assert acc == a ** e, "the Python copy of pow is wrong"
        return res(0, acc), "ok"

    def exp_parse(b):
        m = NUM_RE.fullmatch(b)
        if not m:
            return res(8)
        neg, ip, fp, ex = m.group(1), m.group(2), m.group(3) or b"", m.group(4)
        mant = int(ip + fp)
        scale = -len(fp) + (int(ex) if ex else 0)
        if mant == 0:
            return res(0, Fraction(0))
        while mant % 10 == 0:
            mant //= 10
            scale += 1
        if mant >= 2**64 or scale > 40 or scale < -40:  # a mantissa under 2^64 and a value that fits cannot need an exponent beyond +-40
            return res(15)
        v = Fraction(mant) * Fraction(10) ** scale
        v = -v if neg else v
        return res(0, v) if fits(v) else res(15)

    def exp_dec(fr, places):
        if places > 40:
            return "0|0|"
        x = abs(fr)
        ip, r = divmod(x.numerator, x.denominator)
        digs = ""
        while len(digs) < places and r != 0:
            r *= 10
            digs += str(r // x.denominator)
            r %= x.denominator
        neg = fr < 0 and (ip != 0 or any(c != "0" for c in digs))
        text = ("-" if neg else "") + str(ip) + ("." + digs if digs else "")
        return "%d|%d|%s" % (1 if r == 0 else 0, len(text), text)

    def frac(n, d):
        return Fraction(n, d)

    def rand_frac(bn, bd):
        n = rng.getrandbits(bn) if bn else 0
        d = rng.getrandbits(bd) or 1
        return Fraction(-n if rng.random() < 0.5 else n, d)

    edges = sorted({0, 1, 2, 3, 5, 7, 10, 2**31 - 1, 2**31, 2**31 + 1, 2**32 - 1, 2**32, 2**32 + 1, 3037000499, 3037000500, 2**61, 2**62 - 1, 2**62,
                    2**62 + 1, M - 2, M - 1, M, 3**39, 10**18, M // 3, M // 2, M // 5, M // 7})
    edge_fracs = sorted({Fraction(sg * n, d) for n in edges for d in edges if d >= 1 for sg in (1, -1)})

    pairs = []  # (class, a, b)
    for _ in range(3000):
        pairs.append(("tiny", rand_frac(4, 4), rand_frac(4, 4)))
    for _ in range(4000):
        pairs.append(("small", rand_frac(15, 15), rand_frac(15, 15)))
    for _ in range(4000):
        pairs.append(("mid", rand_frac(31, 31), rand_frac(31, 31)))
    for _ in range(10000):
        pairs.append(("full", rand_frac(rng.randint(1, 63), rng.randint(1, 63)), rand_frac(rng.randint(1, 63), rng.randint(1, 63))))
    for _ in range(6000):
        pairs.append(("edge", rng.choice(edge_fracs), rng.choice(edge_fracs)))
    for _ in range(3000):  # the same big denominator on both sides: the common-factor path of add
        d = rng.getrandbits(rng.randint(20, 63)) | 1 if rng.random() < 0.5 else 1 << rng.randint(20, 62)
        ns = []
        while len(ns) < 2:
            n = rng.getrandbits(rng.randint(18, 63))
            if gcd(n, d) == 1:
                ns.append(-n if rng.random() < 0.5 else n)
        pairs.append(("shared", Fraction(ns[0], d), Fraction(ns[1], d)))
    for cls, a, b in pairs:
        for op in ("add", "sub", "mul", "div"):
            cmds.append("%s %d %d %d %d" % (op, a.numerator, a.denominator, b.numerator, b.denominator))
            if op == "add":
                w, k = exp_addsub(a, b, False)
            elif op == "sub":
                w, k = exp_addsub(a, b, True)
            elif op == "mul":
                w, k = exp_mul(a, b)
            else:
                w, k = exp_div(a, b)
            want.append(w)
            kinds.append((cls, op, k))
        cmds.append("cmp %d %d %d %d" % (a.numerator, a.denominator, b.numerator, b.denominator))
        want.append(str((a > b) - (a < b)))
        kinds.append((cls, "cmp", "ok"))
    n_pairs = len(pairs)

    # power
    exps = [0, 1, 2, 3, 5, 10, 31, 62, 63, 64, 100, -1, -2, -3, -10, -62, -63, INT64_MAX and 2**31 - 1, -2**31]
    for _ in range(6000):
        a = rng.choice(edge_fracs) if rng.random() < 0.3 else rand_frac(rng.randint(1, 8), rng.randint(1, 8))
        e = rng.choice(exps) if rng.random() < 0.5 else rng.randint(-70, 70)
        cmds.append("pow %d %d %d" % (a.numerator, a.denominator, e))
        w, k = exp_pow(a, e)
        want.append(w)
        kinds.append(("pow", "pow", k))
    for a in (Fraction(1), Fraction(-1), Fraction(0), Fraction(2), Fraction(-2), Fraction(1, 2), Fraction(10), Fraction(3)):
        for e in range(-70, 71):
            cmds.append("pow %d %d %d" % (a.numerator, a.denominator, e))
            w, k = exp_pow(a, e)
            want.append(w)
            kinds.append(("pow", "pow", k))

    # unary operations and make
    for _ in range(3000):
        a = rand_frac(rng.randint(1, 63), rng.randint(1, 63))
        cmds.append("neg %d %d" % (a.numerator, a.denominator))
        want.append(res(0, -a))
        kinds.append(("unary", "neg", "ok"))
        cmds.append("abs %d %d" % (a.numerator, a.denominator))
        want.append(res(0, abs(a)))
        kinds.append(("unary", "abs", "ok"))
    makes = edges + [-x for x in edges] + [INT64_MIN, INT64_MIN + 1]
    for _ in range(4000):
        n, d = rng.choice(makes), rng.choice(makes + [rng.getrandbits(20) - (1 << 19)])
        cmds.append("make %d %d" % (n, d))
        if d == 0:
            want.append(res(8))
        elif n == INT64_MIN or d == INT64_MIN:
            want.append(res(15))
        else:
            want.append(res(0, Fraction(n, d)))
        kinds.append(("make", "make", "ok"))

    # operands that are not valid CosRats: every operation refuses them, in either position, and leaves *out alone
    bad = [(1, 0), (0, 2), (2, 4), (1, -1), (INT64_MIN, 1), (5, INT64_MIN), (0, 0), (-3, -4), (6, 9), (0, -1), (INT64_MIN, INT64_MIN), (3, 0), (-1, -1),
           (M, M), (M, -1), (10, 15)]
    for bn, bd in bad:
        for a in (Fraction(1, 2), Fraction(0), Fraction(-7), Fraction(M, 3)):
            for op in ("add", "sub", "mul", "div"):
                cmds.append("%s %d %d %d %d" % (op, bn, bd, a.numerator, a.denominator))
                want.append(res(8))
                kinds.append(("bad", op, "invalid"))
                cmds.append("%s %d %d %d %d" % (op, a.numerator, a.denominator, bn, bd))
                want.append(res(8))
                kinds.append(("bad", op, "invalid"))
        for e in (0, 1, 2, -1):
            cmds.append("pow %d %d %d" % (bn, bd, e))
            want.append(res(8))
            kinds.append(("bad", "pow", "invalid"))
        for op in ("neg", "abs"):
            cmds.append("%s %d %d" % (op, bn, bd))
            want.append(res(8))
            kinds.append(("bad", op, "invalid"))
        cmds.append("fmt %d %d" % (bn, bd))
        want.append("0|")
        kinds.append(("bad", "fmt", "invalid"))
        cmds.append("dec %d %d 5" % (bn, bd))
        want.append("0|0|")
        kinds.append(("bad", "dec", "invalid"))

    # text out
    for _ in range(4000):
        a = rand_frac(rng.randint(1, 63), rng.randint(1, 63)) if rng.random() < 0.8 else rng.choice(edge_fracs)
        cmds.append("fmt %d %d" % (a.numerator, a.denominator))
        t = str(a.numerator) if a.denominator == 1 else "%d/%d" % (a.numerator, a.denominator)
        want.append("%d|%s" % (len(t), t))
        kinds.append(("text", "fmt", "ok"))
        for places in (rng.randint(0, 40), rng.choice([0, 1, 40, 41, 100])):
            cmds.append("dec %d %d %d" % (a.numerator, a.denominator, places))
            want.append(exp_dec(a, places))
            kinds.append(("text", "dec", "ok"))

    # numbers in
    texts = [b"0", b"-0", b"0.0", b"1", b"-1", b"1.5", b"1e3", b"1E-3", b"0e0", b"0e99999999999999999999", b"9223372036854775807", b"9223372036854775808",
             b"-9223372036854775808", b"18446744073709551615", b"18446744073709551616", b"1e18", b"1e19", b"1e-18", b"1e-19", b"0.1", b"01", b"1.", b".5",
             b"-", b"", b"+1", b"1e", b"1e+", b"NaN", b"0x10", b"1 ", b" 1", b"1\x00", b"\xd9\xa1", b"1,5", b"--1", b"1.2.3"]
    mants = [2**64 - 1, 2**64, 2**64 + 1, 2**63 - 1, 2**63, 2**63 + 1, 10**19 - 1, 10**19, 10**19 + 1, 10**18, 10**18 - 1, 2**62, 5**27, 5**28, 2**60 * 5, 3**39,
             M, M - 1, M + 2, 2 * M, 2 * M + 1, 2 * M + 2, 10 * M, 10 * M + 5, 123456789012345678, 1234567890123456789, 12345678901234567890, 123456789012345678901]
    for _ in range(9000):
        mant = rng.choice(mants) if rng.random() < 0.6 else rng.getrandbits(rng.randint(1, 70))
        sdig = str(mant) + "0" * rng.choice([0, 0, 0, 1, 2, 5, 20])
        p = rng.randint(0, len(sdig))
        ip, fp = sdig[:p], sdig[p:]
        if not ip:
            ip = "0"
            fp = "0" * rng.choice([0, 0, 1, 3, 17, 19, 25]) + fp
        if ip.startswith("0") and len(ip) > 1:  # keep the grammar: no leading zeros in the integer part
            ip = ip.lstrip("0") or "0"
        t = ("-" if rng.random() < 0.4 else "") + ip + ("." + fp if fp else "")
        r = rng.random()
        if r < 0.5:
            expo = rng.choice([0, 1, 2, 5, 9, 17, 18, 19, 20, 21, 25, 38, 39, 40, 41, 99, 100000000000000000000]) if rng.random() < 0.7 else rng.randint(0, 60)
            t += rng.choice(["e", "E"]) + rng.choice(["", "+", "-"]) + str(expo)
        texts.append(t.encode())
    junk = [b".", b"e", b"E", b"+", b"-", b"0", b"x", b" ", b"\x00", b"\xd9\xa1", b"/", b"1", b"9", b",", b"_", b"\x7f", b"\xff"]
    for _ in range(4000):
        t = bytearray(rng.choice(texts[: 1 + len(texts) // 2]))
        k = rng.randrange(4)
        if k == 0 and t:
            del t[rng.randrange(len(t))]
        elif k == 1:
            i = rng.randrange(len(t) + 1)
            t[i:i] = rng.choice(junk)
        elif k == 2 and t:
            t = t[: rng.randrange(len(t))]
        elif t:
            i = rng.randrange(len(t))
            t[i : i + 1] = rng.choice(junk)
        texts.append(bytes(t))
    for t in texts:
        cmds.append("parse " + (t.hex() or "-"))
        want.append(exp_parse(t))
        kinds.append(("parse", "parse", "ok"))

    p = run([dump, "rat"], input="\n".join(cmds) + "\n")
    got = p.stdout.splitlines()
    bad_lines = [i for i in range(min(len(got), len(want))) if got[i] != want[i]]
    ok = p.returncode == 0 and len(got) == len(want) and not bad_lines
    detail = "returncode %s, %d answers for %d commands\n" % (p.returncode, len(got), len(want)) + "\n".join(
        "%s -> C says %r, Python says %r" % (cmds[i][:90], got[i], want[i]) for i in bad_lines[:8])
    refused_fit = sum(1 for k in kinds if k[2] == "refused-though-fits")
    ok_count = sum(1 for k in kinds if k[2] == "ok")
    refused = sum(1 for k in kinds if k[2] in ("refused", "refused-though-fits"))
    # the properties that matter on top of equality: operands under 2^31 never reach a refusal in add, sub, mul
    mid_refused = [i for i, k in enumerate(kinds) if k[0] == "mid" and k[1] in ("add", "sub", "mul") and k[2] != "ok"]
    # A refusal of an exactly representable result must be absent for every arithmetic operation.
    wrong_op = [i for i, k in enumerate(kinds) if k[2] == "refused-though-fits" and k[1] not in ("add", "sub")]
    record(ok and not mid_refused and not wrong_op and refused_fit == 0,
           "cross-check: exact rationals equal Python's Fraction on %d commands (%d operand pairs x add/sub/mul/div/cmp, pow, make, neg/abs, "
           "%d number texts, fmt/dec): %d exact answers, %d refusals" % (len(cmds), n_pairs, len(texts), ok_count, refused),
           detail + ("\noperands under 2^31 were refused at %s" % mid_refused[:5] if mid_refused else "") +
           ("\nmul/div refused an answer that fits at %s" % wrong_op[:5] if wrong_op else ""))
    print("      info: %d exactly representable sums were refused" % refused_fit,
          flush=True)


def check_seal_tool():
    import seal
    errors = []

    def must_fail(mutate, what):
        spec = {"build_id": "b", "articles": [{"id": 1, "title": "t", "text": "x"}, {"id": 2, "title": "u", "text": "y"}],
                "methods": [{"id": 1, "name": "a.b", "flags": ["json"], "max_len": 5}, {"id": 2, "name": "c.d"}]}
        mutate(spec)
        try:
            seal.digest(spec)
            errors.append("accepted: " + what)
        except seal.SpecError:
            pass

    must_fail(lambda s: s["articles"][0].__setitem__("text", "a\x00b"), "NUL inside text (the C core would stop reading at it)")
    must_fail(lambda s: s["articles"][1].__setitem__("id", 1), "duplicate article id")
    must_fail(lambda s: s["methods"][1].__setitem__("id", 1), "duplicate method id")
    must_fail(lambda s: s["methods"].reverse(), "method ids not increasing")
    must_fail(lambda s: s["methods"][0].__setitem__("flags", ["nope"]), "unknown flag")
    must_fail(lambda s: s["methods"][0].__setitem__("name", "Bad Name"), "bad method name")
    must_fail(lambda s: s["methods"][0].__setitem__("id", 256), "method id above 255")
    must_fail(lambda s: s.__setitem__("build_id", ""), "empty build id")
    must_fail(lambda s: s["articles"][0].__setitem__("title", "\ud800"), "lone surrogate in text")
    # the limits parsed from the header are the ones the compiler sees
    prog = "#include <stdio.h>\n#include \"cos_limits.h\"\nint main(void){printf(\"%u %u %u %u %u %u %u %u %u\\n\"," + ",".join(
        "(unsigned)" + n for n in seal.LIMIT_NAMES) + ");return 0;}\n"
    src = os.path.join(BUILD, "limits_probe.c")
    open(src, "w").write(prog)
    exe = os.path.join(BUILD, "limits_probe")
    p = run(["gcc", "-std=c17", "-I" + C17, src, "-o", exe])
    if p.returncode != 0:
        errors.append("limits probe did not build: " + p.stderr[:300])
    else:
        got = [int(x) for x in run([exe]).stdout.split()]
        if got != seal.read_limits():
            errors.append("tools/seal.py reads limits %s but the compiler sees %s" % (seal.read_limits(), got))
    record(not errors, "tools/seal.py: refuses malformed specs (9 cases) and reads the same nine limits the compiler sees", "\n".join(errors))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--quick", action="store_true", help="tests at -O2 only")
    args = ap.parse_args()
    os.makedirs(OBJ, exist_ok=True)
    check_source_rules()
    check_compile()
    check_headers()
    check_objects()
    import check_riscv
    check_riscv.run_checks(record)
    import check_rv  # the assembly layer against LLVM's assembler and disassembler (the independent oracle)
    check_rv.run_checks(record, quick=args.quick)
    import check_alg  # the number tower of the Work-Time Engine against the independent pair-rule oracle (Python Fractions)
    check_alg.run_checks(record, quick=args.quick)
    import check_mat  # exact matrix elimination against an independent Fraction oracle
    check_mat.run_checks(record, quick=args.quick)
    check_seal_tool()
    seal_vector()
    dump = build_dump()
    check_hash(dump)
    check_cpp_sha()
    check_unicode(dump)
    check_rat(dump)
    check_tests(args.quick)
    bad = [r for r in results if not r[0]]
    print("\n%s: %d of %d checks passed" % ("ALL OK" if not bad else "FAILED", len(results) - len(bad), len(results)))
    return 0 if not bad else 1


if __name__ == "__main__":
    sys.exit(main())
