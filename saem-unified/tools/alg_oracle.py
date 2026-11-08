"""Independent oracle for core/c17/cos_alg.c (the Work-Time Engine's number tower).

Cayley-Dickson algebras over the rationals, built ONLY by the pair rule on Python Fractions, with no sign table and no basis-index trick, so that it
shares no code and no idea with the C kernel (which multiplies by signed basis products found by a four-case rule):

    (a, b)(c, d) = (a c - conj(d) b,  d a + b conj(c)),        conj(a, b) = (conj(a), -b)         [this project's convention]

An element of level L is a list of 2**L Fractions. Baez, "The Octonions" (Bull. AMS 39, 2002) section 2.2 uses the mirror convention
(a, b)(c, d) = (a c - d conj(b), conj(a) d + c b) (his equations (2) and (3)); mul_baez below implements it, and `to_baez` is the explicit isomorphism
(a, b) -> (to_baez(a), conj(to_baez(b))) between the two, checked in tools/check_alg.py, so the kernel agrees with the published definition up to a
named isomorphism. Python is test tooling only: the kernel does not use it.
"""
from fractions import Fraction as F

MAX_KERNEL_LEVEL = 3


def dim(level):
    return 1 << level


def zero(level):
    return [F(0)] * dim(level)


def one(level):
    v = zero(level)
    v[0] = F(1)
    return v


def basis(level, i):
    v = zero(level)
    v[i] = F(1)
    return v


def conj(a):
    if len(a) == 1:
        return list(a)
    h = len(a) // 2
    return conj(a[:h]) + [-x for x in a[h:]]


def add(a, b):
    return [x + y for x, y in zip(a, b)]


def sub(a, b):
    return [x - y for x, y in zip(a, b)]


def neg(a):
    return [-x for x in a]


def scale(s, a):
    return [s * x for x in a]


def mul(a, b):
    n = len(a)
    if n == 1:
        return [a[0] * b[0]]
    h = n // 2
    a1, a2, b1, b2 = a[:h], a[h:], b[:h], b[h:]
    return sub(mul(a1, b1), mul(conj(b2), a2)) + add(mul(b2, a1), mul(a2, conj(b1)))


def mul_baez(a, b):
    n = len(a)
    if n == 1:
        return [a[0] * b[0]]
    h = n // 2
    a1, a2, b1, b2 = a[:h], a[h:], b[:h], b[h:]
    return sub(mul_baez(a1, b1), mul_baez(b2, conj(a2))) + add(mul_baez(conj(a1), b2), mul_baez(b1, a2))


def to_baez(a):
    """The isomorphism from this project's convention to Baez's: (a, b) -> (to_baez(a), conj(to_baez(b)))."""
    if len(a) == 1:
        return list(a)
    h = len(a) // 2
    return to_baez(a[:h]) + conj(to_baez(a[h:]))


def real_part(a):
    return a[0]


def norm2(a):
    """The sum of squares of the components (the kernel's definition); tests assert it equals the real part of a * conj(a)."""
    return sum((x * x for x in a), F(0))


def inv(a):
    n = norm2(a)
    if n == 0:
        raise ZeroDivisionError("inverse of zero")
    return [x / n for x in conj(a)]


def is_zero(a):
    return all(x == 0 for x in a)


def assoc(a, b, c):
    return sub(mul(mul(a, b), c), mul(a, mul(b, c)))


def comm(a, b):
    return sub(mul(a, b), mul(b, a))


def basis_sign_and_index(level, i, j):
    """e_i e_j = sign e_k, found by multiplying the basis elements with the pair rule (used to check the kernel's sign function)."""
    p = mul(basis(level, i), basis(level, j))
    nz = [(k, v) for k, v in enumerate(p) if v != 0]
    assert len(nz) == 1 and abs(nz[0][1]) == 1, "a basis product must be a signed basis element"
    return int(nz[0][1]), nz[0][0]


def find_zero_divisor(level, bound=1, limit=2000000):
    """Search for nonzero a, b with a b = 0 using integer components in [-bound, bound] on few basis elements (sparse: two or four terms each).
    Returns (a, b) or None. Used for level 4 (the sedenions): the reason the kernel's tower stops at level 3."""
    n = dim(level)
    import itertools
    seen = 0
    pairs = [(i, j, s) for i in range(n) for j in range(i + 1, n) for s in (1, -1)]
    for (i, j, s) in pairs:
        a = zero(level)
        a[i] = F(1)
        a[j] = F(s)
        for (k, l, t) in pairs:
            b = zero(level)
            b[k] = F(1)
            b[l] = F(t)
            seen += 1
            if seen > limit:
                return None
            if is_zero(mul(a, b)):
                return a, b
    return None
