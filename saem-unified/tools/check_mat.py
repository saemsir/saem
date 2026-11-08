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

def oracle_solve(matrix, rows, cols, rhs):
    augmented = [matrix[row * cols:(row + 1) * cols] + [rhs[row]] for row in range(rows)]
    pivots, reduced = oracle([x for row in augmented for x in row], rows, cols + 1)
    rref = [reduced[i * (cols + 1):(i + 1) * (cols + 1)] for i in range(rows)]
    for row in range(rows):
        if all(rref[row][col] == 0 for col in range(cols)) and rref[row][cols] != 0:
            return None, None
    solution = [Fraction(0) for _ in range(cols)]
    rank = 0
    for row, col in enumerate(pivots):
        if col == cols:
            continue
        solution[col] = rref[row][cols]
        rank += 1
    return rank, solution

def oracle_projection(matrix, rows, cols):
    pivots, _ = oracle(matrix, rows, cols)
    basis = [[matrix[row * cols + col] for row in range(rows)] for col in pivots]
    rank = len(basis)
    if rank == 0:
        return [Fraction(0) for _ in range(rows * rows)]
    gram = [[sum(basis[i][row] * basis[j][row] for row in range(rows))
             for j in range(rank)] for i in range(rank)]
    inverse = []
    for col in range(rank):
        rhs = [Fraction(int(row == col)) for row in range(rank)]
        _, solution = oracle_solve([x for row in gram for x in row], rank, rank, rhs)
        inverse.append(solution)
    return [sum(basis[k][row] * inverse[l][k] * basis[l][col_row]
                for k in range(rank) for l in range(rank))
            for row in range(rows) for col_row in range(rows)]


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

    solve_exe = os.path.join(BUILD, "c17_mat_solve_dump")
    solve_cmd = ["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                 "-I" + C17, os.path.join(TESTS, "c17_mat_solve_dump.c"),
                 os.path.join(C17, "cos_mat.c"), os.path.join(C17, "cos_rat.c"), "-o", solve_exe]
    built = subprocess.run(solve_cmd, capture_output=True, text=True)
    if built.returncode:
        raise RuntimeError(built.stderr)
    solve_cases = []
    for _ in range(250):
        rows, cols = rng.randint(1, 6), rng.randint(1, 6)
        matrix = [Fraction(rng.randint(-5, 5), rng.randint(1, 5)) for _ in range(rows * cols)]
        if rng.random() < 0.6:
            source = [Fraction(rng.randint(-4, 4), rng.randint(1, 4)) for _ in range(cols)]
            rhs = [sum(matrix[row * cols + col] * source[col] for col in range(cols)) for row in range(rows)]
        else:
            rhs = [Fraction(rng.randint(-5, 5), rng.randint(1, 5)) for _ in range(rows)]
        solve_cases.append((rows, cols, matrix, rhs))
    solve_lines, solve_expected = [], []
    for rows, cols, matrix, rhs in solve_cases:
        rank, solution = oracle_solve(matrix, rows, cols, rhs)
        values = matrix + rhs
        solve_lines.append("s %d %d %s" % (rows, cols, " ".join(
            "%d %d" % (x.numerator, x.denominator) for x in values)))
        solve_expected.append((rank, solution))
    solve_run = subprocess.run([solve_exe], input="\n".join(solve_lines) + "\n",
                               capture_output=True, text=True)
    solve_answers = solve_run.stdout.splitlines()
    solve_errors = []
    if solve_run.returncode or len(solve_answers) != len(solve_cases):
        solve_errors.append("dump returned %d and %d lines for %d cases: %s" %
                            (solve_run.returncode, len(solve_answers), len(solve_cases), solve_run.stderr[:300]))
    else:
        for index, (line, (rank, solution)) in enumerate(zip(solve_answers, solve_expected)):
            tokens = line.split()
            status = int(tokens[0])
            if rank is None:
                if status != 4:
                    solve_errors.append("case %d expected inconsistent/COS_E_NOT_FOUND, got %s" % (index, line[:100]))
            elif status != 0:
                solve_errors.append("case %d unexpectedly refused: %s" % (index, line[:100]))
            else:
                got_rank = int(tokens[1])
                got = [Fraction(int(x.split("/")[0]), int(x.split("/")[1])) for x in tokens[2:]]
                if got_rank != rank or got != solution:
                    solve_errors.append("case %d mismatch: rank %d/%d solution %s/%s" %
                                        (index, got_rank, rank, got[:8], solution[:8]))
            if len(solve_errors) == 8:
                break
    record(not solve_errors, "linear solve: %d exact/inconsistent systems agree with Fraction oracle" % len(solve_cases),
           "\n".join(solve_errors))

    inverse_cases = []
    for _ in range(100):
        n = rng.randint(1, 6)
        matrix = [Fraction(rng.randint(-5, 5), rng.randint(1, 5)) for _ in range(n * n)]
        augmented = [matrix[row * n:(row + 1) * n] +
                     [Fraction(int(row == col)) for col in range(n)] for row in range(n)]
        pivots, reduced = oracle([x for row in augmented for x in row], n, 2 * n)
        invertible = len([col for col in pivots if col < n]) == n
        inverse = [reduced[row * 2 * n + n + col] for row in range(n) for col in range(n)] if invertible else None
        inverse_cases.append((n, matrix, inverse))
    inverse_lines, inverse_expected = [], []
    for n, matrix, inverse in inverse_cases:
        inverse_lines.append("i %d %s" % (n, " ".join(
            "%d %d" % (x.numerator, x.denominator) for x in matrix)))
        inverse_expected.append(inverse)
    inverse_run = subprocess.run([solve_exe], input="\n".join(inverse_lines) + "\n",
                                 capture_output=True, text=True)
    inverse_answers = inverse_run.stdout.splitlines()
    inverse_errors = []
    if inverse_run.returncode or len(inverse_answers) != len(inverse_cases):
        inverse_errors.append("dump returned %d and %d lines for %d cases: %s" %
                              (inverse_run.returncode, len(inverse_answers), len(inverse_cases), inverse_run.stderr[:300]))
    else:
        for index, (line, want) in enumerate(zip(inverse_answers, inverse_expected)):
            tokens = line.split()
            status = int(tokens[0])
            if want is None:
                if status != 4:
                    inverse_errors.append("case %d expected singular/COS_E_NOT_FOUND, got %s" % (index, line[:100]))
            elif status != 0:
                inverse_errors.append("case %d unexpectedly refused: %s" % (index, line[:100]))
            else:
                got = [Fraction(int(x.split("/")[0]), int(x.split("/")[1])) for x in tokens[1:]]
                if got != want:
                    inverse_errors.append("case %d inverse mismatch: %s/%s" % (index, got[:8], want[:8]))
            if len(inverse_errors) == 8:
                break
    record(not inverse_errors, "inverse: %d exact/singular matrices agree with Fraction oracle" % len(inverse_cases),
           "\n".join(inverse_errors))

    projection_exe = os.path.join(BUILD, "c17_mat_projection_dump")
    projection_cmd = ["gcc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                      "-I" + C17, os.path.join(TESTS, "c17_mat_projection_dump.c"),
                      os.path.join(C17, "cos_mat.c"), os.path.join(C17, "cos_rat.c"), "-o", projection_exe]
    built = subprocess.run(projection_cmd, capture_output=True, text=True)
    if built.returncode:
        raise RuntimeError(built.stderr)
    projection_cases = []
    for _ in range(150):
        rows, cols = rng.randint(1, 5), rng.randint(1, 5)
        matrix = [Fraction(rng.randint(-3, 3), rng.randint(1, 4)) for _ in range(rows * cols)]
        projection_cases.append((rows, cols, matrix, oracle_projection(matrix, rows, cols)))
    projection_lines = []
    for rows, cols, matrix, _ in projection_cases:
        projection_lines.append("%d %d %s" % (rows, cols, " ".join(
            "%d %d" % (x.numerator, x.denominator) for x in matrix)))
    projection_run = subprocess.run([projection_exe], input="\n".join(projection_lines) + "\n",
                                    capture_output=True, text=True)
    projection_answers = projection_run.stdout.splitlines()
    projection_errors = []
    if projection_run.returncode or len(projection_answers) != len(projection_cases):
        projection_errors.append("dump returned %d and %d lines for %d cases: %s" %
                                 (projection_run.returncode, len(projection_answers),
                                  len(projection_cases), projection_run.stderr[:300]))
    else:
        for index, (line, case) in enumerate(zip(projection_answers, projection_cases)):
            status, *tokens = line.split()
            got = [Fraction(int(x.split("/")[0]), int(x.split("/")[1])) for x in tokens] if int(status) == 0 else None
            if int(status) != 0 or got != case[3]:
                projection_errors.append("case %d projection mismatch: status %s" % (index, status))
                if len(projection_errors) == 8:
                    break
    record(not projection_errors, "projection: %d exact matrices agree with Fraction oracle" % len(projection_cases),
           "\n".join(projection_errors))
    return not errors and not space_errors and not solve_errors and not inverse_errors and not projection_errors


def main():
    def report(ok, title, detail=""):
        print("%s  %s" % ("ok  " if ok else "FAIL", title))
        if detail:
            print("      " + detail.replace("\n", "\n      "))

    return 0 if run_checks(report) else 1


if __name__ == "__main__":
    raise SystemExit(main())
