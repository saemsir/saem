#!/usr/bin/env python3
"""Proves the Work-Time Engine's number tower (core/c17/cos_alg.c) against an independent oracle: Python Fractions and the Cayley-Dickson pair rule
(tools/alg_oracle.py), which shares no code and no idea with the kernel (the kernel multiplies by signed basis products found by a four-case rule).

    python3 tools/check_alg.py            full run (well under a minute)
    python3 tools/check_alg.py --quick    a quarter of the cases and no mutation run

  oracle      the oracle itself obeys the theorems before it is trusted: associative at levels 0..2, alternative with the three Moufang identities and
              norm2(ab) = norm2(a) norm2(b) at level 3, and NOT associative at level 3 (a witness); Baez's mirror convention is isomorphic to ours
  products    add, sub, mul, neg, conj, inv, scale, norm2 and both regular-representation matrices, levels 0..3, three operand classes (small, mid, wide):
              the kernel's answer is EXACTLY the oracle's, or it refuses with COS_E_RANGE and leaves the output untouched, and never anything else; for
              the small class and for the operations that cannot overflow it must answer. A rounded or wrapped value is therefore impossible to hide.
  signs       the basis sign table of every level 0..3 against products found by the pair rule
  refusals    level 4 and above (the sedenions) are refused by everything; malformed operands (a zero or negative denominator, an unreduced fraction,
              INT64_MIN, a nonzero unused component) are refused with COS_E_INVALID and the output is left alone
  baez        the kernel's product, carried through the explicit isomorphism, is the product of Baez's published definition (levels 1..3)
  sedenions   why the tower stops at level 3: an explicit pair of nonzero sedenions with product 0, and the norm rule failing at levels 4 and 5
  mutation    twenty-odd seeded faults in cos_alg.c (a flipped sign, a swapped operand order, a dropped check, an aliasing bug, ...): the C test suite
              must fail on every one, so a green run means something
Exit code 0 only if everything passes."""
import argparse
import os
import random
import subprocess
import sys
import tempfile
from fractions import Fraction as F

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C17 = os.path.join(ROOT, "core", "c17")
TESTS = os.path.join(ROOT, "core", "tests")
BUILD = os.path.join(ROOT, "build_c17")
sys.path.insert(0, os.path.join(ROOT, "tools"))
import alg_oracle as A  # noqa: E402

I64MAX = 2 ** 63 - 1
SENT = "12345/678"          # what c17_alg_dump.c puts in every output slot before the call
ST_INVALID, ST_RANGE = 8, 15  # cos_status.h
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


def build_dump():
    os.makedirs(BUILD, exist_ok=True)
    exe = os.path.join(BUILD, "c17_alg_dump")
    p = run(["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Werror", "-I" + C17, os.path.join(TESTS, "c17_alg_dump.c"),
             os.path.join(C17, "cos_alg.c"), os.path.join(C17, "cos_rat.c"), "-o", exe])
    if p.returncode != 0:
        raise RuntimeError(p.stderr)
    return exe


def batch(dump, lines):
    p = run([dump], input="\n".join(lines) + "\n")
    out = p.stdout.splitlines()
    if p.returncode != 0 or len(out) != len(lines):
        raise RuntimeError("c17_alg_dump failed: code %s, %d answers for %d commands\n%s" % (p.returncode, len(out), len(lines), p.stderr[:500]))
    return out


# ------------------------------------------------------------------------------------------------------------------ operands
def tok(x):
    return "%d/%d" % (x.numerator, x.denominator)


def pad(vec):
    return list(vec) + [F(0)] * (8 - len(vec))


def toks(vec):
    return " ".join(tok(x) for x in pad(vec))


def rep(x):
    """Does the exact rational fit cos_rat (int64 numerator without INT64_MIN, positive int64 denominator)?"""
    return -I64MAX <= x.numerator <= I64MAX and x.denominator <= I64MAX


def rnd_rat(rng, cls):
    if cls == "small":
        return F(rng.randint(-6, 6), rng.choice((1, 2, 3, 4, 6)))
    if cls == "mid":
        return F(rng.randint(-10 ** 6, 10 ** 6), rng.choice((1, 2, 3, 5, 7, 11)))
    return F(rng.randint(-2 ** 40, 2 ** 40), rng.randint(1, 2 ** 20))


def rnd_elem(rng, level, cls):
    v = [rnd_rat(rng, cls) for _ in range(1 << level)]
    return [x if rng.random() > 0.3 else F(0) for x in v]  # sparse on purpose: the engine mostly holds sparse elements, and it exercises the zero skip


def to_frac(t):
    n, d = t.split("/")
    return F(int(n), int(d))


def judge(line, want, must, slots):
    """None if the kernel's answer is acceptable, else why not. want: the oracle's exact result (a list of Fractions), must: it has to answer."""
    t = line.split()
    st, comps = int(t[0]), t[1:]
    if len(comps) != slots:
        return "malformed answer: " + line
    if st == 0:
        got = [to_frac(c) for c in comps]
        if any(c != tok(g) for c, g in zip(comps, got)):
            return "not in canonical form: " + line
        if got != want[:slots]:
            return "WRONG VALUE: got %s, oracle %s" % ([tok(g) for g in got], [tok(w) for w in want[:slots]])
        return None
    if must:
        return "refused with status %d but the exact answer fits and the operands are small" % st
    if st != ST_RANGE:
        return "status %d, a valid call may only answer or refuse with COS_E_RANGE (%d)" % (st, ST_RANGE)
    if comps != [SENT] * slots:
        return "refused but the output was touched: " + line
    return None


class Group:
    """Commands and what each must answer; run in one batch; one report line."""

    def __init__(self, name):
        self.name, self.lines, self.wants, self.musts, self.slots = name, [], [], [], []

    def add(self, line, want, must, slots=8):
        self.lines.append(line)
        self.wants.append(want)
        self.musts.append(must)
        self.slots.append(slots)

    def run(self, dump):
        outs = batch(dump, self.lines)
        bad, exact, refused = [], 0, 0
        for i, line in enumerate(outs):
            why = judge(line, self.wants[i], self.musts[i], self.slots[i])
            if why:
                bad.append("%s :: %s" % (self.lines[i][:140], why))
            elif line.split()[0] == "0":
                exact += 1
            else:
                refused += 1
        record(not bad, "%-9s %5d cases: %d exact, %d refused as out of range, none wrong or touched" % (self.name, len(outs), exact, refused), "\n".join(bad))
        return outs


# ------------------------------------------------------------------------------------------------------------------ the cross-check
def check_products(dump, quick):
    rng = random.Random(20261105)
    n = 40 if quick else 160
    classes = ("small", "mid", "wide")
    g = {k: Group(k) for k in ("add", "sub", "mul", "neg", "conj", "inv", "scale", "norm2", "lmat", "rmat")}
    baez = []  # (level, a, b, c-line) for the kernel-versus-Baez check
    for L in range(4):
        d = 1 << L
        for cls in classes:
            for _ in range(n):
                a, b = rnd_elem(rng, L, cls), rnd_elem(rng, L, cls)
                for name, fn in (("add", A.add), ("sub", A.sub), ("mul", A.mul)):
                    want = pad(fn(a, b))
                    g[name].add("%s %d %s %s" % (name, L, toks(a), toks(b)), want, cls == "small")
                g["neg"].add("neg %d %s" % (L, toks(a)), pad(A.neg(a)), True)       # cannot overflow: no INT64_MIN is ever produced
                g["conj"].add("conj %d %s" % (L, toks(a)), pad(A.conj(a)), True)
                s = rnd_rat(rng, cls)
                g["scale"].add("scale %d %s %s" % (L, tok(s), toks(a)), pad(A.scale(s, a)), cls == "small")
                g["norm2"].add("norm2 %d %s" % (L, toks(a)), [A.norm2(a)], cls == "small", 1)
                if not A.is_zero(a):
                    g["inv"].add("inv %d %s" % (L, toks(a)), pad(A.inv(a)), cls == "small")
                if cls == "small":
                    baez.append((L, a, b))
            # the regular representations: column col of L_a is a e_col and column col of R_b is e_col b (independent of the kernel's sign rule)
            a = rnd_elem(rng, L, "wide")
            for r in range(d):
                for c in range(d):
                    ec = A.basis(L, c)
                    g["lmat"].add("lmat %d %s %d %d" % (L, toks(a), r, c), [A.mul(a, ec)[r]], True, 1)
                    g["rmat"].add("rmat %d %s %d %d" % (L, toks(a), r, c), [A.mul(ec, a)[r]], True, 1)
    for k in ("add", "sub", "mul", "neg", "conj", "inv", "scale", "norm2", "lmat", "rmat"):
        g[k].run(dump)
    # the kernel's product carried through psi is Baez's product: the published definition up to a named isomorphism
    lines = ["mul %d %s %s" % (L, toks(a), toks(b)) for L, a, b in baez if L >= 1]
    outs = batch(dump, lines)
    bad = []
    for (L, a, b), line in zip([x for x in baez if x[0] >= 1], outs):
        t = line.split()
        c = [to_frac(x) for x in t[1:1 + (1 << L)]]
        if t[0] != "0" or A.to_baez(c) != A.mul_baez(A.to_baez(a), A.to_baez(b)):
            bad.append("level %d, a=%s b=%s" % (L, [tok(x) for x in a], [tok(x) for x in b]))
    record(not bad, "baez      %5d products at levels 1..3: kernel product through psi = Baez's (a,b)(c,d) = (ac - d b*, a* d + c b)" % len(lines), "\n".join(bad[:10]))


def check_signs(dump):
    lines, want = [], []
    for L in range(4):
        d = 1 << L
        for i in range(d):
            for j in range(d):
                s, k = A.basis_sign_and_index(L, i, j)
                assert k == i ^ j
                lines.append("sign %d %d %d" % (L, i, j))
                want.append(s)
        lines += ["sign %d %d %d" % (L, d, 0), "sign %d %d %d" % (L, 0, d), "sign %d %d %d" % (L, 255, 255)]
        want += [0, 0, 0]
    for L in (4, 5, 31, 2 ** 32 - 1):
        lines.append("sign %d 0 0" % L)
        want.append(0)
    outs = batch(dump, lines)
    bad = ["%s -> %s, oracle %d" % (ln, o, w) for ln, o, w in zip(lines, outs, want) if int(o) != w]
    record(not bad, "signs     %5d entries: sign(i, j) of e_i e_j = sign e_(i xor j) at levels 0..3 equals the pair-rule product; out of range gives 0" % len(lines),
           "\n".join(bad[:10]))


def check_refusals(dump):
    rng = random.Random(77)
    e1 = toks(A.basis(3, 1))
    bad = []
    lines, want = [], []
    # level 4 and above: dimension 0 and every operation refuses, output untouched
    for L in (4, 5, 6, 7, 8, 31, 2 ** 32 - 1):
        a = toks(rnd_elem(rng, 3, "small"))
        cmds = ["dim %d" % L, "add %d %s %s" % (L, a, e1), "sub %d %s %s" % (L, a, e1), "mul %d %s %s" % (L, a, e1), "neg %d %s" % (L, a),
                "conj %d %s" % (L, a), "inv %d %s" % (L, e1), "scale %d 1/2 %s" % (L, a), "norm2 %d %s" % (L, a), "lmat %d %s 0 0" % (L, a),
                "rmat %d %s 0 0" % (L, a), "valid %d %s" % (L, a), "eq %d %s %s" % (L, a, a), "zero %d %s" % (L, a), "embed %d 3 %s" % (L, a),
                "embed 0 %d %s" % (L, a)]
        lines += cmds
        want += ["0"] + ["%d %s" % (ST_INVALID, " ".join([SENT] * (1 if c.startswith(("norm2", "lmat", "rmat")) else 8))) for c in cmds[1:9] + cmds[9:11]] + \
                ["0", "0", "0", "%d %s" % (ST_INVALID, " ".join([SENT] * 8)), "%d %s" % (ST_INVALID, " ".join([SENT] * 8))]
    outs = batch(dump, lines)
    for ln, o, w in zip(lines, outs, want):
        if o != w:
            bad.append("%s -> %s, expected %s" % (ln[:90], o, w))
    record(not bad, "refusals  %5d calls at levels 4, 5, 6, 7, 8, 31, 2^32-1 (the sedenions and beyond): dim 0, COS_E_INVALID, output untouched" % len(lines),
           "\n".join(bad[:10]))
    # malformed operands: refused with COS_E_INVALID, output untouched; canonical operands are reported valid
    good = [F(1, 2), F(-3, 4), F(0), F(5), F(1, 3), F(-2), F(7, 9), F(0)]
    badtoks = ["1/0", "1/-2", "2/4", "-9223372036854775808/1", "0/5", "0/0", "3/-1", "4/6"]
    lines2, want2 = [], []
    for L in range(4):
        d = 1 << L
        for pos in range(8):
            for bt in badtoks:
                v = [tok(x) for x in good[:d]] + ["0/1"] * (8 - d)
                v[pos] = bt
                el = " ".join(v)
                clean = toks(good[:d])
                lines2 += ["add %d %s %s" % (L, el, clean), "mul %d %s %s" % (L, clean, el), "conj %d %s" % (L, el), "inv %d %s" % (L, el),
                           "norm2 %d %s" % (L, el), "valid %d %s" % (L, el)]
                w8 = "%d %s" % (ST_INVALID, " ".join([SENT] * 8))
                want2 += [w8, w8, w8, w8, "%d %s" % (ST_INVALID, SENT), "0"]
        # a nonzero unused component (positions >= d) is not canonical; positions < d with a good value are
        lines2.append("valid %d %s" % (L, toks(good[:d])))
        want2.append("1")
    outs = batch(dump, lines2)
    bad = ["%s -> %s, expected %s" % (ln[:100], o, w) for ln, o, w in zip(lines2, outs, want2) if o != w]
    record(not bad, "malformed %5d calls: a zero/negative denominator, an unreduced fraction, INT64_MIN or a nonzero unused slot -> COS_E_INVALID, untouched"
           % len(lines2), "\n".join(bad[:10]))
    # embed, dim, eq, zero
    rng2 = random.Random(5)
    lines3, want3 = [], []
    for L in range(4):
        lines3.append("dim %d" % L)
        want3.append(str(1 << L))
        for to in range(4):
            for _ in range(8):
                a = rnd_elem(rng2, L, "wide")
                lines3.append("embed %d %d %s" % (L, to, toks(a)))
                want3.append("0 " + toks(a) if L <= to else "%d %s" % (ST_INVALID, " ".join([SENT] * 8)))
        a, b = rnd_elem(rng2, L, "small"), rnd_elem(rng2, L, "small")
        lines3 += ["eq %d %s %s" % (L, toks(a), toks(a)), "eq %d %s %s" % (L, toks(a), toks(b)), "zero %d %s" % (L, toks(a)), "zero %d %s" % (L, toks([F(0)] * (1 << L)))]
        want3 += ["1", "1" if a == b else "0", "1" if A.is_zero(a) else "0", "1"]
    outs = batch(dump, lines3)
    bad = ["%s -> %s, expected %s" % (ln[:100], o, w) for ln, o, w in zip(lines3, outs, want3) if o != w]
    record(not bad, "embed     %5d calls: dim = 2^level, the inclusion copies components upward and refuses downward, eq and zero are exact" % len(lines3),
           "\n".join(bad[:10]))


# ------------------------------------------------------------------------------------------------------------------ the oracle's own theorems, sedenions
def check_oracle_theorems(quick):
    rng = random.Random(1105)
    n = 40 if quick else 150

    def rnd(level):
        return [F(rng.randint(-5, 5), rng.choice((1, 2, 3))) for _ in range(1 << level)]

    bad = []
    for L in range(0, 3):
        for _ in range(n):
            a, b, c = rnd(L), rnd(L), rnd(L)
            if not A.is_zero(A.assoc(a, b, c)):
                bad.append("associativity fails at level %d" % L)
    e1, e2, e4 = A.basis(3, 1), A.basis(3, 2), A.basis(3, 4)
    if A.is_zero(A.assoc(e1, e2, e4)) or A.assoc(e1, e2, e4) != A.scale(F(-2), A.mul(e1, A.mul(e2, e4))):
        bad.append("the level-3 non-associativity witness (e1 e2) e4 = - e1 (e2 e4) is not reproduced")
    for _ in range(n):
        a, b, c = rnd(3), rnd(3), rnd(3)
        m = A.mul
        if m(m(a, a), b) != m(a, m(a, b)) or m(m(b, a), a) != m(b, m(a, a)) or m(m(a, b), a) != m(a, m(b, a)):
            bad.append("alternativity or flexibility fails at level 3")
        if m(m(c, a), m(b, c)) != m(m(c, m(a, b)), c) or m(c, m(a, m(c, b))) != m(m(m(c, a), c), b) or m(a, m(c, m(b, c))) != m(m(m(a, c), b), c):
            bad.append("a Moufang identity fails at level 3")
        if A.norm2(m(a, b)) != A.norm2(a) * A.norm2(b):
            bad.append("norm2(ab) != norm2(a) norm2(b) at level 3")
        if A.is_zero(a) or A.is_zero(b):
            continue
        if A.is_zero(m(a, b)):
            bad.append("a zero divisor at level 3")
    record(not bad, "oracle    the pair-rule oracle obeys the theorems it is trusted for: associative at levels 0..2; at level 3 non-associative but alternative, "
           "flexible, Moufang, norm-multiplicative, no zero divisors (%d random triples per level)" % n, "\n".join(sorted(set(bad))))
    bad = []
    for L in range(1, 5):
        for _ in range(n):
            a, b = rnd(L), rnd(L)
            if A.to_baez(A.mul(a, b)) != A.mul_baez(A.to_baez(a), A.to_baez(b)):
                bad.append("psi is not a homomorphism at level %d" % L)
    record(not bad, "baez      psi (a, b) -> (psi a, conj psi b) carries this convention onto Baez's at levels 1..4 (%d random pairs each)" % n, "\n".join(sorted(set(bad))))
    # the sedenions: the reason the tower stops
    a, b = A.zero(4), A.zero(4)
    a[1], a[10] = F(1), F(1)
    b[4], b[15] = F(1), F(-1)
    fails = {}
    for L in range(0, 6):
        fails[L] = sum(A.norm2(A.mul(x, y)) != A.norm2(x) * A.norm2(y) for x, y in ((rnd(L), rnd(L)) for _ in range(n)))
    ok = A.is_zero(A.mul(a, b)) and not A.is_zero(a) and not A.is_zero(b) and A.norm2(a) * A.norm2(b) == 4 and all(fails[L] == 0 for L in range(4)) and \
        fails[4] > 0 and fails[5] > 0
    record(ok, "sedenions (e1 + e10)(e4 - e15) = 0 with both factors nonzero (norms 2 and 2, so norm2(ab) = 0 != 4); norm2(ab) != norm2(a) norm2(b) in %d/%d "
           "random pairs at level 4 and %d/%d at level 5, and in none at levels 0..3: the tower must stop at the octonions" % (fails[4], n, fails[5], n))


# ------------------------------------------------------------------------------------------------------------------ mutation self-test
MUTATIONS = [
    ("the conjugation sign of imaginary units is lost", [("return k == 0u ? 1 : -1;", "return 1;")]),
    ("the sign of the (1,1) Cayley-Dickson case is flipped", [("s *= jb ? -chi(lo_j) : chi(lo_j);", "s *= jb ? chi(lo_j) : chi(lo_j);")]),
    ("the pair is not swapped when the second top bit is set", [("i = jb ? lo_j : lo_i;\n    j = jb ? lo_i : lo_j;", "i = lo_i;\n    j = lo_j;")]),
    ("the factor is also applied in the (0,1) case", [("if (ib) s *= jb", "if (ib || jb) s *= jb")]),
    ("level 3 is multiplied with the level-2 rule", [("for (uint32_t l = level; l > 0u; --l) {", "for (uint32_t l = level > 2u ? 2u : level; l > 0u; --l) {")]),
    ("the product reverses its operand order", [("if (sgn(level, i, j) < 0) {", "if (sgn(level, j, i) < 0) {")]),
    ("the zero-skip drops factors equal to 1", [("if (b->c[j].num == 0) continue;", "if (b->c[j].num == 1) continue;")]),
    ("an overflow in the accumulation is ignored", [("st = cos_rat_add(*slot, p, slot);\n      if (st != COS_OK) return st;", "st = cos_rat_add(*slot, p, slot);\n      (void)st;")]),
    ("the product writes into its output while reading its inputs", [
        ("  CosAlg r;\n  cos_alg_zero(&r);\n  for (uint32_t i = 0; i < d; ++i) {\n    if (a->c[i].num == 0) continue;",
         "  CosAlg *rp = out;\n  cos_alg_zero(rp);\n  for (uint32_t i = 0; i < d; ++i) {\n    if (a->c[i].num == 0) continue;"),
        ("CosRat *slot = &r.c[i ^ j];", "CosRat *slot = &rp->c[i ^ j];"),
        ("  *out = r;\n  return COS_OK;\n}\n\nCosStatus cos_alg_conj", "  return COS_OK;\n}\n\nCosStatus cos_alg_conj")]),
    ("conjugation leaves e_1 alone", [("for (uint32_t k = 1; k < d; ++k) {\n    const CosStatus st = cos_rat_neg(a->c[k], &r.c[k]);", "for (uint32_t k = 2; k < d; ++k) {\n    const CosStatus st = cos_rat_neg(a->c[k], &r.c[k]);")]),
    ("the norm multiplies by the real part instead of squaring", [("cos_rat_mul(a->c[k], a->c[k], &sq)", "cos_rat_mul(a->c[k], a->c[0], &sq)")]),
    ("the inverse forgets to conjugate e_1", [("if (k > 0u) {\n      st = cos_rat_neg(v, &v);", "if (k > 1u) {\n      st = cos_rat_neg(v, &v);")]),
    ("the inverse of zero reports the wrong status", [("if (n2.num == 0) return COS_E_INVALID;", "if (n2.num == 0) return COS_E_RANGE;")]),
    ("subtraction adds and addition subtracts", [("sub ? cos_rat_sub(a->c[k], b->c[k], &r.c[k]) : cos_rat_add(a->c[k], b->c[k], &r.c[k])",
                                                  "sub ? cos_rat_add(a->c[k], b->c[k], &r.c[k]) : cos_rat_sub(a->c[k], b->c[k], &r.c[k])")]),
    ("scaling adds the scalar", [("cos_rat_mul(s, a->c[k], &r.c[k])", "cos_rat_add(s, a->c[k], &r.c[k])")]),
    ("the unused components are not checked for canonical form", [("    } else if (a->c[k].num != 0 || a->c[k].den != 1) {\n      return false;\n    }\n", "    }\n")]),
    ("the basis element e_i is built at index 0", [("r.c[i].num = 1;", "r.c[0].num = 1;")]),
    ("from_ints accepts more than dim integers", [("n > d || ", "")]),
    ("the embedding accepts a downward map", [("from > to || ", "")]),
    ("the embedding does not check its operand", [(" || !cos_alg_valid(from, a)) return COS_E_INVALID;\n  *out = *a;", ") return COS_E_INVALID;\n  *out = *a;")]),
    ("the left matrix uses the wrong index", [("const uint32_t i = row ^ col; /* e_i e_col = sign e_row */", "const uint32_t i = row & col; /* e_i e_col = sign e_row */")]),
    ("the right matrix transposes the sign", [("if (sgn(level, col, j) > 0) {", "if (sgn(level, j, col) > 0) {")]),
    ("cos_alg_zero faults on NULL", [("  if (out == NULL) return; /* a void function cannot report it, but it must not fault: the Forbidden Zone is fail-closed, never a crash */\n", "")]),
]


def check_mutations():
    src = open(os.path.join(C17, "cos_alg.c"), encoding="utf-8").read()
    survived, unbuilt, stale = [], [], []
    with tempfile.TemporaryDirectory() as td:
        for name, edits in MUTATIONS:
            s = src
            ok = True
            for old, new in edits:
                if s.count(old) != 1:
                    stale.append("%s: edit text found %d times (must be once): %r" % (name, s.count(old), old[:60]))
                    ok = False
                    break
                s = s.replace(old, new)
            if not ok:
                continue
            c = os.path.join(td, "cos_alg_mut.c")
            with open(c, "w", encoding="utf-8", newline="\n") as f:
                f.write(s)
            exe = os.path.join(td, "t_mut")
            p = run(["gcc", "-std=c17", "-O1", "-w", "-I" + C17, "-I" + TESTS, os.path.join(TESTS, "c17_alg_test.c"), c, os.path.join(C17, "cos_rat.c"), "-o", exe])
            if p.returncode != 0:
                unbuilt.append("%s: %s" % (name, p.stderr.strip().splitlines()[0] if p.stderr.strip() else "?"))
                continue
            try:
                q = run([exe], timeout=60)
                tail = (q.stdout + q.stderr).strip().splitlines()
                last = tail[-1] if tail else ""
                caught = q.returncode != 0 or " 0 failed" not in last
            except subprocess.TimeoutExpired:
                caught = True  # a hang is also a failure of the suite's subject
            if not caught:
                survived.append(name)
    total = len(MUTATIONS)
    bad = ["SURVIVED (no test noticed): " + x for x in survived] + ["did not build: " + x for x in unbuilt] + ["stale edit: " + x for x in stale]
    record(not bad, "mutation  %d of %d seeded faults in cos_alg.c are caught by the C test suite" % (total - len(survived) - len(unbuilt) - len(stale), total), "\n".join(bad))


# ------------------------------------------------------------------------------------------------------------------
def run_checks(rec, quick=False):
    """Entry point for tools/check_c17.py: the same proofs, reported through its record(ok, title, detail)."""
    global record
    saved = record
    record = lambda ok, title, detail="": rec(ok, "alg " + title, detail)  # noqa: E731
    try:
        run_all(quick)
    finally:
        record = saved


def run_all(quick):
    check_oracle_theorems(quick)
    dump = build_dump()
    check_signs(dump)
    check_products(dump, quick)
    check_refusals(dump)
    if not quick:
        check_mutations()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--quick", action="store_true", help="a quarter of the cases, no mutation run")
    args = ap.parse_args()
    run_all(args.quick)
    bad = results.count(False)
    print("\n%s: %d of %d checks passed" % ("ALL OK" if not bad else "FAILED", len(results) - bad, len(results)))
    return 0 if not bad else 1


if __name__ == "__main__":
    sys.exit(main())
