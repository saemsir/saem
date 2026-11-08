# Linux in QCOS: what it is for, what it may never decide, what we take from it

Written 2026-11-01. Evidence: the Linux tree in `/home/claude/torvalds/linux` (version 7.3.0, commit 36aad21d, 2026-10-30; a PARTIAL clone: `net/core/*`, `drivers/remoteproc`, `drivers/rpmsg` are absent),
Torvalds' thesis (`linus.pdf`, Helsinki 1997), the project's virtio-v1.1. Companion: `docs/RISCV_QCOS.md` (the hardware side and the door), `docs/knowledge/linux.md` (per-topic notes).
Legend: **[V]** we read the file at the path and line given; **[U]** unverified here (the file is not in the local clone, or we only know it from documentation); **[P]** planned.

## 1. The rule

Linux supplies what a sovereign core should not reinvent: drivers, file systems, networking, scheduling, the user interface stack, and a huge tested code base.
Linux never supplies a DECISION. Every decision (gate verdict, quarantine, release, freeze) is taken by the sovereign kernel (`core/c17/`) on the monitor core from its own sealed tables and written to its own hash-chained journal first: "no receipt, no action".
Linux is the host, the courier and the display. If it lies, crashes or is compromised, QCOS refuses or freezes; it does not guess.

Torvalds' thesis supports the split: the virtual-machine layer so common code never sees hardware (sections 1 and 2.4); Linux running ON TOP of another kernel is an established design (MkLinux and L4Linux, section 1);
policy in user-level daemons, mechanism in the kernel; and, for Phase 2 and 4, massively parallel machines as one OS image per node with message passing in user space (4.4.2): our "cohesive team of model nodes".

## 2. Role by phase

| Phase | Where Linux runs | What the QCOS core is | Door |
|---|---|---|---|
| 1 home hardware (phone, laptop, desktop) | Android and desktop Linux are Linux; Windows, macOS and iOS are not | the C17 library inside the app process (default profile if several threads call it; `COS_SINGLE_HART=1` only if the app serializes every call on one thread). `qcosd` is optional on Linux and Android | function calls through the C header; JSON methods |
| 2 enterprise parallel | one Linux per cluster node; GPUs through Linux DRM and the LLVM AMDGPU/NVPTX backends [P] | one sovereign core per node; nodes talk by messages, not shared memory; SQLite L3 and RDMA | sockets or RDMA between `qcosd` peers; the gate sits in front |
| 3 custom silicon 4+1 | Linux in S-mode on the four application harts | the kernel on the monitor hart in M-mode, `COS_SINGLE_HART=1` | shared-memory rings plus doorbells (`RISCV_QCOS.md` section 4) |
| 4 air-gapped datacentre | the same Linux, built from a pinned tree, no external network | as phase 3, per node | internal fabric only |
| 5 P2P marketplace | Linux nodes of independent parties | contracts (Al-Ujrah, Al-Musharakah, Al-Istisna') are sealed articles; Zone 0 pass C checks each offer; no interest-bearing term can pass | signed messages |

iOS has no Linux: there the core runs in-process and the iOS sandbox is the outer wall. [U] `ios-security-may12.pdf` is in the project and must be read before any claim about it.

## 3. What each Linux mechanism gives us (all [V] unless marked)

| Mechanism | Where we read it | The QCOS invariant it serves | Decision |
|---|---|---|---|
| `SECCOMP_RET_KILL_PROCESS` as the default action | `include/uapi/linux/seccomp.h:40`; `kernel/seccomp.c:416, 1038, 1362` | fail closed: an unlisted syscall kills the process, it is not skipped | ADOPT for `qcosd` |
| `PR_SET_NO_NEW_PRIVS` | `include/uapi/linux/prctl.h:175`; `kernel/sys.c:2709`; required for unprivileged seccomp (`Documentation/userspace-api/seccomp_filter.rst:71`) and for Landlock (`Documentation/userspace-api/landlock.rst:350`) | privilege can only shrink | ADOPT |
| Landlock: create ruleset, add rule, restrict self | `security/landlock/syscalls.c:230, 661, 732` | deny by default file access without root | ADOPT |
| memfd seals `F_SEAL_SEAL`, `F_SEAL_WRITE`, `F_SEAL_FUTURE_WRITE` | `include/uapi/linux/fcntl.h:47, 50, 51` | an input that cannot change after it was verified (the sealed constitution, a forecast snapshot) | ADOPT |
| New network namespace | `include/uapi/linux/sched.h:33` `CLONE_NEWNET` | `qcosd` has no network | ADOPT |
| `mlock` | `mm/mlock.c:654` | keys never reach swap | ADOPT |
| `SO_PEERCRED`, `setrlimit` | `net/core/sock.c` is NOT in the local clone | who is on the other end of the socket; memory and CPU ceilings | [U] re-add the file or read the man pages before relying |
| io_uring "start disabled, register restrictions, then enable" | `io_uring/register.c:167` `io_register_restrictions`, `:277` `io_register_enable_rings`; `include/uapi/linux/io_uring.h:181`; checked with `smp_load_acquire` at `io_uring/io_uring.c:2869` | the door's lifecycle: nothing flows before the allow-list is in place | ADAPT |
| io_uring shared rings | `include/linux/io_uring_types.h:194` `struct io_rings` (free-running `u32` head and tail masked by `entries - 1`, one writer per counter, explicit `sq_dropped` and `cq_overflow`, the application may rewrite pending completion entries) | the door's ring layout; "shared memory is input, never state" | ADAPT |
| virtio rings | `include/uapi/linux/virtio_ring.h:104, 111, 128` | Phase 2 transport option | ADAPT [P] |
| Taint word | `include/linux/panic.h:72` to `:93` (`TAINT_PROPRIETARY_MODULE` = bit 0 up to `TAINT_FLAGS_COUNT` = 20); `kernel/panic.c:954` `add_taint`; `Documentation/admin-guide/tainted-kernels.rst:85` | a sticky word whose bits only turn on: our freeze-reason set. Today the core keeps ONE reason, "first reason wins" (`cos_core_freeze`); a multi-reason sticky word is planned | ADAPT [P] |
| Lockdown | `security/lockdown/lockdown.c:84, 167`; levels `include/linux/security.h:109` to `:151` | two named levels, integrity and confidentiality, that only rise | ADAPT |
| Integrity measurement, verity, module signing, audit | `security/integrity/ima/ima_main.c:254` `process_measurement`; `drivers/md/dm-verity-target.c`; `fs/verity/verify.c`; `kernel/module/signing.c`; `kernel/audit.c:1913` `audit_log_start` | verify before use, journal what happened. Our hash chain already does this; no module loading into the monitor, ever | study only |
| Lock-free building blocks | `lib/kfifo.c:114, 180` (`__kfifo_in`, `__kfifo_out`); `lib/llist.c:31` `llist_del_first`; `include/linux/seqlock.h:407` | single-producer ring and lock-free list: our pool and rings are the same family. A seqlock's reader retry suits a counter, not the door | study |
| Memory model | `Documentation/memory-barriers.txt` (`smp_load_acquire`, `smp_store_release`); `tools/memory-model/litmus-tests/` | write litmus tests for the door rings in that style [P] | ADAPT |
| Overflow-checked arithmetic | `include/linux/overflow.h:61, 153` (`check_add_overflow`, `check_mul_overflow`) | refuse instead of wrapping: the same rule `cos_rat` follows (RANGE) | ADOPT (already) |
| Strict number parsing | `lib/kstrtox.c:135, 159` | whole string or error; our `cos_rat` parser does the same | ADOPT (already) |
| Binary GCD | `lib/math/gcd.c` (header comment: binary GCD) | exact rationals reduce by gcd | study |
| `rational_best_approximation` | `lib/math/rational.c:35` | it APPROXIMATES a fraction with bounded numerator and denominator: exactly what "zero numerical approximation" forbids | **REFUSE** |
| Test fixtures | `tools/testing/selftests/kselftest_harness.h:219, 237, 267` | fixture style for host tests | optional |

## 4. The `qcosd` sandbox recipe (design [P]; each step is a mechanism read above)

In this order, all before the first byte of input is read, in a process that has no other job:
1. `prctl(PR_SET_NO_NEW_PRIVS, 1)`.
2. A new network namespace with no interface (`CLONE_NEWNET`).
3. A Landlock ruleset: read-only on the engine directory and the sealed data, write only on one scratch directory, then `restrict_self`.
4. `mlock` the key pages; set rlimits [U] on memory, CPU and open files.
5. Inputs arrive as sealed memfds (`F_SEAL_SEAL | F_SEAL_WRITE`), so what the gate verified is what the engine reads.
6. A seccomp filter whose default is `SECCOMP_RET_KILL_PROCESS`, allowing only read, write, the door's doorbell call, `mmap` of the sealed fds, futex, exit.
7. Peer identity by `SO_PEERCRED` [U] on the local socket; nothing else may connect.
The daemon talks to the sovereign core only through the door and trusts nothing it returns that the core did not sign into the journal.

## 5. What QCOS refuses from Linux

- Any pointer, length or index it hands us: arithmetic validity is checked by the core itself (the FU540 errata ROCK-2 says even a hardware trap cannot be relied on for a wild pointer).
- Time: the journal's `tick` is a logical time the caller gives; the core never reads a clock.
- Dynamic code in the monitor: no modules, no JIT, no loader. The sealed image is the image.
- Network access for `qcosd`.
- Numerical approximation of any kind in the symbolic path (`rational_best_approximation`, floating point).
- A decision that depends on Linux telling the truth.

## 6. Verified, unverified, planned

Verified today: every `path:line` marked [V] above was read in the tree on 2026-11-01; the kernel's behaviour under both atomic profiles is proven in `docs/RISCV_QCOS.md` section 2.
Unverified: `SO_PEERCRED`, `setrlimit`, remoteproc and rpmsg (their files are not in the local clone; re-attach `torvalds/linux` with `add_repo` for a full tree), anything about iOS.
Planned: the door, `qcosd`, the sticky taint word, door litmus tests, virtio transport, GPU path.
