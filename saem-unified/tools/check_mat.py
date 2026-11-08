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
    def value(x):
        return x if isinstance(x, Fraction) else Fraction(x[0], x[1])

    a = [[value(matrix[r * cols + c]) for c in range(cols)] for r in range(rows)]
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

def oracle_spaces(matrix, rows, cols):
    pivots, reduced = oracle(matrix, rows, cols)
    rref = [reduced[i * cols:(i + 1) * cols] for i in range(rows)]
    pivot_set = set(pivots)
    null_basis = []
    for free_col in range(cols):
        if free_col in pivot_set:
            continue
        vector = [Fraction(0) for _ in range(cols)]
        vector[free_col] = Fraction(1)
        for row, pivot_col in enumerate(pivots):
            vector[pivot_col] = -rref[row][free_col]
        null_basis.extend(vector)
    row_basis = [value for row in rref[:len(pivots)] for value in row]
    column_basis = [matrix[row * cols + col] for col in pivots for row in range(rows)]
    transposed = [matrix[row * cols + col] for col in range(cols) for row in range(rows)]
    left_pivots, left_reduced = oracle(transposed, cols, rows)
    left_rref = [left_reduced[i * rows:(i + 1) * rows] for i in range(cols)]
    left_pivot_set = set(left_pivots)
    left_basis = []
    for free_col in range(rows):
        if free_col in left_pivot_set:
            continue
        vector = [Fraction(0) for _ in range(rows)]
        vector[free_col] = Fraction(1)
        for row, pivot_col in enumerate(left_pivots):
            vector[pivot_col] = -left_rref[row][free_col]
        left_basis.extend(vector)
    return (len(null_basis) // cols, len(row_basis) // cols,
            len(column_basis) // rows, len(left_basis) // rows,
            null_basis + row_basis + column_basis + left_basis)


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

    spaces_exe = os.path.join(BUILD, "c17_mat_spaces_dump")
    spaces_cmd = ["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-I" + C17, os.path.join(TESTS, "c17_mat_spaces_dump.c"),
                  os.path.join(C17, "cos_mat.c"), os.path.join(C17, "cos_rat.c"), "-o", spaces_exe]
    built = subprocess.run(spaces_cmd, capture_output=True, text=True)
    if built.returncode:
        raise RuntimeError(built.stderr)
    space_lines = []
    space_expected = []
    for rows, cols, matrix in cases:
        canonical = [Fraction(n, d) for n, d in matrix]
        space_lines.append("%d %d %s" % (rows, cols, " ".join(
            "%d %d" % (x.numerator, x.denominator) for x in canonical)))
        space_expected.append(oracle_spaces(canonical, rows, cols))
    space_run = subprocess.run([spaces_exe], input="\n".join(space_lines) + "\n",
                               capture_output=True, text=True)
    space_answers = space_run.stdout.splitlines()
    space_errors = []
    if space_run.returncode or len(space_answers) != len(cases):
        space_errors.append("dump returned %d and %d lines for %d cases: %s" %
                            (space_run.returncode, len(space_answers), len(cases), space_run.stderr[:300]))
    else:
        for index, (line, want) in enumerate(zip(space_answers, space_expected)):
            tokens = line.split()
            if not tokens or int(tokens[0]) != 0:
                space_errors.append("case %d refused with %s" % (index, tokens[0] if tokens else "no output"))
                continue
            dims = tuple(int(x) for x in tokens[1:5])
            got = [Fraction(int(x.split("/")[0]), int(x.split("/")[1])) for x in tokens[5:]]
            if dims != want[:4] or got != want[4]:
                space_errors.append("case %d mismatch: dims %s/%s" % (index, dims, want[:4]))
                if len(space_errors) == 8:
                    break
    record(not space_errors, "four subspaces: %d exact basis sets agree with the independent Fraction oracle" % len(cases),
           "\n".join(space_errors))
    return not errors and not space_errors


def main():
    def report(ok, title, detail=""):
        print("%s  %s" % ("ok  " if ok else "FAIL", title))
        if detail:
            print("      " + detail.replace("\n", "\n      "))

    return 0 if run_checks(report) else 1


if __name__ == "__main__":
    raise SystemExit(main())
