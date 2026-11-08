# What Linux 7.3.0 teaches QCOS

Source tree read: `/home/claude/torvalds/linux` (Makefile version 7.3.0, commit 36aad21d, 2026-10-30). All paths below are relative to that tree unless marked `[qcos]`.
Evidence rule: every claim carries `path:line` read in this session. NOT FOUND means we looked and it is not there. Where memory and the file disagree, the file wins (see "Surprises").
Form per topic: Linux does / For QCOS / Decision (ADOPT, ADAPT, REFUSE) / Build.

## Topic list (status, updated 2026-11-01)

The design that uses these notes is `docs/LINUX_QCOS.md` (role of Linux, mechanism table with `path:line`, the `qcosd` sandbox recipe, what QCOS refuses);
the door and the hardware side are in `docs/RISCV_QCOS.md`. This file stays the per-topic evidence log. The local clone is PARTIAL: `net/core/*`,
`drivers/remoteproc/*` and `drivers/rpmsg/*` are NOT in it (NOT FOUND, looked 2026-11-01); re-attach `torvalds/linux` with `add_repo` for a full tree.

1. The door to a non-Linux core (rpmsg, remoteproc, virtio ring, reserved memory)  - PARTIAL. Read: io_uring shared rings (`include/linux/io_uring_types.h:194` `struct io_rings`:
   free-running u32 head and tail, one writer per counter, `sq_dropped`, `cq_overflow`), virtio rings (`include/uapi/linux/virtio_ring.h:104/111/128`). NOT FOUND: rpmsg, remoteproc.
2. Sealing the door (io_uring restrictions, enable path, full completion ring)  - DONE. `io_uring/register.c:167` `io_register_restrictions`, `:277` `io_register_enable_rings`,
   `include/uapi/linux/io_uring.h:181` `IORING_SETUP_R_DISABLED`, checked with `smp_load_acquire` at `io_uring/io_uring.c:2869`. Decision: ADAPT as the door lifecycle (create disabled, restrict, enable).
3. Gate and sandbox for `qcosd` (seccomp, no_new_privs, Landlock, namespaces, memfd seals)  - DONE except BPF, LSM stacking, AF_UNIX (PENDING) and `SO_PEERCRED`/`setrlimit` (NOT FOUND in the clone).
   `include/uapi/linux/seccomp.h:40`, `kernel/seccomp.c:416/1038/1362`, `include/uapi/linux/prctl.h:175`, `kernel/sys.c:2709`, `security/landlock/syscalls.c:230/661/732`,
   `include/uapi/linux/fcntl.h:47/50/51`, `include/uapi/linux/sched.h:33`, `mm/mlock.c:654`. Decision: ADOPT, in the order of `docs/LINUX_QCOS.md` section 4.
4. Tamper evidence and failure policy (IMA, dm-verity, fs-verity, module signing, panic/taint, audit)  - PARTIAL. Taint word read (`include/linux/panic.h:72-93`, `kernel/panic.c:954`,
   `Documentation/admin-guide/tainted-kernels.rst:85`); lockdown levels located (`security/lockdown/lockdown.c:84/167`, `include/linux/security.h:109-151`); IMA, verity, signing, audit located
   (`security/integrity/ima/ima_main.c:254`, `drivers/md/dm-verity-target.c`, `fs/verity/verify.c`, `kernel/module/signing.c`, `kernel/audit.c:1913`) but not yet read in depth.
5. Lock-free and memory order (barriers, llist, kfifo, seqlock, LKMM litmus tests)  - PARTIAL. Located: `lib/kfifo.c:114/180`, `lib/llist.c:31`, `include/linux/seqlock.h:407`,
   `Documentation/memory-barriers.txt`, `tools/memory-model/litmus-tests/`. The LKMM itself and the litmus tests are PENDING; plan: door-ring litmus tests in that style.
6. Input discipline (kstrtox, overflow.h, gcd, rational, div64, utf8, warning flags, kselftest harness)  - PARTIAL. `include/linux/overflow.h:61/153`, `lib/kstrtox.c:135/159`, `lib/math/gcd.c`
   (binary GCD), `lib/math/rational.c:35` `rational_best_approximation` (an APPROXIMATION: decision REFUSE), `tools/testing/selftests/kselftest_harness.h:219/237/267`. PENDING: div64, utf8, warning flags.

(Surprises and open questions come last.)

## Surprises

- `lib/math/rational.c` exists to find the best fraction with a bounded numerator and denominator. That is the opposite of QCOS's rule (exact or refuse), so QCOS reads it only to know what NOT to do.
- The local Linux clone has `io_uring/` and `security/` but not `net/core/`: a sparse checkout. Any claim about sockets must be marked unverified until the file is present.

## Open questions

- Which seccomp allow-list is the smallest that still lets `qcosd` run the engines (to be found by running them under a trace on a real Linux)?
- Does the door need a doorbell system call at all in Phase 1 (polling on the Linux side may be enough), or only in Phase 3 (`msip` and PLIC)?
