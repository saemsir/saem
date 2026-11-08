/* The ONLY place the sovereign kernel touches a hardware atomic. Two profiles, chosen at compile time with -DCOS_SINGLE_HART=0|1.
 * The memory layout is identical in both (every shared word is an `_Atomic uint32_t` or `_Atomic uint64_t`), so objects, tests and tools
 * see the same structures; only the instructions differ.
 *
 *   COS_SINGLE_HART 0 (default)   Many harts or threads may call the kernel at once. Each operation is the C11 atomic with the memory order
 *                                 written at the call site. On RV64 that is lr.d/sc.d or amo* plus fence; elsewhere the native lock-free form.
 *   COS_SINGLE_HART 1             Exactly one hart ever runs kernel code: the "+1" monitor core. Each operation is a relaxed load, a computation
 *                                 and a relaxed store. No lr, no sc, no amo and no fence is emitted (tools/check_riscv.py proves it from the
 *                                 object code). A relaxed aligned load or store is one instruction and single-copy atomic (RISC-V unprivileged
 *                                 manual, RVWMO, section 17.1), which is all one hart needs.
 *
 * Why the second profile exists (evidence, read 2026-10-31 and 2026-11-01):
 *   - SiFive FU540-C000 manual, section 3.5: the E51 monitor core has no data cache; lr and sc raise an access exception on its DTIM and on every
 *     other uncached region. AMOs are supported there, but the same manual's errata ROCK-3 says their acquire/release ordering is wrong on the
 *     E51. So on the monitor core the one safe atomic is the plain aligned load or store.
 *   - RISC-V unprivileged manual 13.3: lr/sc loops are only guaranteed to make progress when constrained; tools/check_riscv.py checks that
 *     the default profile's loops obey the constraints, and the single-hart profile has no loops at all.
 *   - Linus Torvalds' thesis (Helsinki 1997), 5.1: the Alpha port broke code that used a byte as an atomic flag, because the smallest atomic
 *     unit there was a word. Every word we share is therefore 32 or 64 bits wide, never smaller (the journal lock was an atomic_flag byte;
 *     it is now a 32-bit word).
 *
 * Contract of COS_SINGLE_HART 1 (the caller guarantees it; the kernel detects what it cheaply can):
 *   - One thread of control is inside the kernel at any time.
 *   - An interrupt handler may set a flag the platform layer owns. It never calls the kernel and never touches kernel memory.
 *   - A block-device callback that calls back into the same journal is refused with COS_E_BUSY and changes nothing; it is never waited for,
 *     because waiting for yourself is a hang with no diagnosis.
 *   - The door to other harts (shared-memory rings) is NOT kernel state. It is its own module, uses store-release and load-acquire, and
 *     may emit fence (the E51 executes fence); it never uses lr, sc or amo. Its operations will be added to this header as a third group.
 *
 * Rules for kernel sources (tools/check_c17.py enforces them): no direct atomic_* call, no atomic_flag and no <stdatomic.h> outside this
 * header; memory orders stay written at each call site (they document what the site needs, and the default profile compiles them as written);
 * a new operation is added to BOTH profiles in the same change.
 * C17 only: _Atomic and _Generic do not exist in C++ (the C++ engines meet this code through the extern "C" door, not through this header). */
#ifndef COS_ATOMIC_H
#define COS_ATOMIC_H

#ifdef __cplusplus
#error "cos_atomic.h is C17 only; C++ code reaches the kernel through its extern \"C\" interface"
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef COS_SINGLE_HART
#define COS_SINGLE_HART 0
#endif
#if COS_SINGLE_HART != 0 && COS_SINGLE_HART != 1
#error "COS_SINGLE_HART must be 0 (many harts) or 1 (exactly one hart)"
#endif

/* Initialise an atomic word that no other hart can see yet (memory just cleared by the caller). A plain store in both profiles. */
#define cos_init(p, v) atomic_init((p), (v))

#if COS_SINGLE_HART == 0

/* ---- many harts: the C11 operations, one to one, memory order as written ---- */
#define cos_load(p, mo) atomic_load_explicit((p), (mo))
#define cos_store(p, v, mo) atomic_store_explicit((p), (v), (mo))
#define cos_fetch_add(p, v, mo) atomic_fetch_add_explicit((p), (v), (mo))
#define cos_fetch_sub(p, v, mo) atomic_fetch_sub_explicit((p), (v), (mo))
#define cos_fetch_or(p, v, mo) atomic_fetch_or_explicit((p), (v), (mo))
#define cos_fetch_and(p, v, mo) atomic_fetch_and_explicit((p), (v), (mo))
#define cos_exchange(p, v, mo) atomic_exchange_explicit((p), (v), (mo))
/* Compare and swap: true when *p equalled *e and now holds d; false when not, and then *e holds what was found. */
#define cos_cas(p, e, d, ok, fail) atomic_compare_exchange_strong_explicit((p), (e), (d), (ok), (fail))

#else

/* ---- one hart: relaxed load, compute, relaxed store. The memory-order arguments are accepted and ignored (there is nobody to order against).
 * Each helper is a few plain instructions. _Generic picks the 32-bit or the 64-bit helper from the pointer type, so a word of any other type
 * (or a plain non-atomic word) is a compile error, in this profile and, because the tests build both, before it can reach the default one. ---- */
#define COS_SH_RMW(NAME, T, EXPR)                                         \
  static inline T NAME(_Atomic T *p, T v) {                               \
    const T old = atomic_load_explicit(p, memory_order_relaxed);          \
    atomic_store_explicit(p, (T)(EXPR), memory_order_relaxed);            \
    return old;                                                           \
  }
COS_SH_RMW(cos_sh_add32, uint32_t, old + v)
COS_SH_RMW(cos_sh_add64, uint64_t, old + v)
COS_SH_RMW(cos_sh_sub32, uint32_t, old - v)
COS_SH_RMW(cos_sh_sub64, uint64_t, old - v)
COS_SH_RMW(cos_sh_or32, uint32_t, old | v)
COS_SH_RMW(cos_sh_or64, uint64_t, old | v)
COS_SH_RMW(cos_sh_and32, uint32_t, old & v)
COS_SH_RMW(cos_sh_and64, uint64_t, old & v)
COS_SH_RMW(cos_sh_xchg32, uint32_t, v)
COS_SH_RMW(cos_sh_xchg64, uint64_t, v)
#undef COS_SH_RMW

#define COS_SH_CAS(NAME, T)                                               \
  static inline bool NAME(_Atomic T *p, T *e, T d) {                      \
    const T old = atomic_load_explicit(p, memory_order_relaxed);          \
    if (old != *e) {                                                      \
      *e = old;                                                           \
      return false;                                                       \
    }                                                                     \
    atomic_store_explicit(p, d, memory_order_relaxed);                    \
    return true;                                                          \
  }
COS_SH_CAS(cos_sh_cas32, uint32_t)
COS_SH_CAS(cos_sh_cas64, uint64_t)
#undef COS_SH_CAS

#define COS_SH_PICK(p, F32, F64) _Generic((p), _Atomic uint32_t *: F32, _Atomic uint64_t *: F64)

#define cos_load(p, mo) atomic_load_explicit((p), memory_order_relaxed)
#define cos_store(p, v, mo) atomic_store_explicit((p), (v), memory_order_relaxed)
#define cos_fetch_add(p, v, mo) COS_SH_PICK((p), cos_sh_add32, cos_sh_add64)((p), (v))
#define cos_fetch_sub(p, v, mo) COS_SH_PICK((p), cos_sh_sub32, cos_sh_sub64)((p), (v))
#define cos_fetch_or(p, v, mo) COS_SH_PICK((p), cos_sh_or32, cos_sh_or64)((p), (v))
#define cos_fetch_and(p, v, mo) COS_SH_PICK((p), cos_sh_and32, cos_sh_and64)((p), (v))
#define cos_exchange(p, v, mo) COS_SH_PICK((p), cos_sh_xchg32, cos_sh_xchg64)((p), (v))
#define cos_cas(p, e, d, ok, fail) COS_SH_PICK((p), cos_sh_cas32, cos_sh_cas64)((p), (e), (d))

#endif /* COS_SINGLE_HART */
#endif /* COS_ATOMIC_H */
