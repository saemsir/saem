#!/usr/bin/env python3
"""Check exact C17 matrix RREF against an independent Fraction elimination."""
import os
import random
import subprocess

from fractions import Fraction

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
C17 = os.path.join(ROOT, "core", "c17")
TESTS = os.path.join(ROOT, "core", "tests")
BUILD = os.path.join(ROOT, "build_c17")
MAX = 2**63 - 1


def oracle(matrix, rows, cols):
    a = [[Fraction(matrix[r * cols + c][0], matrix[r * cols + c][1]) for c in range(cols)] for r in range(rows)]
    pivots = []
    pivot_row = 0
    for col in range(cols):
        found = next((r for r in range(pivot_row, rows) if a[r][col]), rows)
        if found == rows:
            continue
        a[pivot_row], a[found] = a[found], a[pivot_row]
        pivot = a[pivot_row][col]
        a[pivot_row] = [x / pivot for x in a[pivot_row]]
        for r in range(rows):
            if r != pivot_row and a[r][col]:
                factor = a[r][col]
                a[r] = [x - factor * y for x, y in zip(a[r], a[pivot_row])]
        pivots.append(col)
        pivot_row += 1
        if pivot_row == rows:
            break
    return pivots, [x for row in a for x in row]


def fits(x):
    return -MAX <= x.numerator <= MAX and x.denominator <= MAX


def run_checks(record, quick=False):
    os.makedirs(BUILD, exist_ok=True)
    exe = os.path.join(BUILD, "c17_mat_dump")
    cmd = ["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
           "-I" + C17, os.path.join(TESTS, "c17_mat_dump.c"),
           os.path.join(C17, "cos_mat.c"), os.path.join(C17, "cos_rat.c"), "-o", exe]
    built = subprocess.run(cmd, capture_output=True, text=True)
    if built.returncode:
        raise RuntimeError(built.stderr)

    rng = random.Random(20261108)
    cases = [
        (3, 3, [(1, 1), (2, 1), (1, 1), (2, 1), (4, 1), (0, 1), (0, 1), (0, 1), (1, 1)]),
        (2, 3, [(0, 1), (0, 1), (1, 1), (0, 1), (0, 1), (0, 1)]),
        (3, 2, [(1, 2), (1, 3), (2, 3), (1, 2), (1, 1), (5, 6)]),
    ]
    for _ in range(100 if quick else 500):
        rows, cols = rng.randint(1, 6), rng.randint(1, 6)
        matrix = [(rng.randint(-5, 5), rng.randint(1, 5)) for _ in range(rows * cols)]
        cases.append((rows, cols, matrix))

    lines = []
    expected = []
    for rows, cols, matrix in cases:
        pivots, result = oracle(matrix, rows, cols)
        representable = all(fits(x) for x in result)
        canonical = [Fraction(n, d) for n, d in matrix]
        lines.append("%d %d %s" % (rows, cols, " ".join("%d %d" % (x.numerator, x.denominator) for x in canonical)))
        expected.append((pivots, result, representable))
    run = subprocess.run([exe], input="\n".join(lines) + "\n", capture_output=True, text=True)
    answers = run.stdout.splitlines()
    errors = []
    if run.returncode or len(answers) != len(cases):
        errors.append("dump returned %d and %d lines for %d cases: %s" %
                      (run.returncode, len(answers), len(cases), run.stderr[:300]))
    else:
        for index, (line, (pivots, values, representable)) in enumerate(zip(answers, expected)):
            tokens = line.split()
            status = int(tokens[0])
            if status != 0:
                errors.append("case %d unexpectedly refused with status %d" % (index, status))
                continue
            rank = int(tokens[1])
            got_pivots = [int(x) for x in tokens[2:2 + rank]]
            got_values = [Fraction(int(x.split("/")[0]), int(x.split("/")[1])) for x in tokens[2 + rank:]]
            if not representable or got_pivots != pivots or got_values != values:
                errors.append("case %d mismatch: pivots %s/%s values %s/%s" %
                              (index, got_pivots, pivots, got_values[:8], values[:8]))
                if len(errors) == 8:
                    break
    record(not errors, "matrix RREF: %d exact cases agree with the independent Fraction oracle" % len(cases),
           "\n".join(errors))
    return not errors


def main():
    def report(ok, title, detail=""):
        print("%s  %s" % ("ok  " if ok else "FAIL", title))
        if detail:
            print("      " + detail.replace("\n", "\n      "))

    return 0 if run_checks(report) else 1


if __name__ == "__main__":
    raise SystemExit(main())
