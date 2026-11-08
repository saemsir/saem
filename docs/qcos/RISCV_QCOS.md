# QCOS on RISC-V: the "4+1" mapping, the monitor core, the two atomic profiles

Written 2026-11-01 from the project books (`riscv-privileged.pdf`, `riscv-unprivileged.pdf`, both ratified 20250508; `FU540-C000-v1.0.pdf`; `linus.pdf`)
and from objects we built and read in this tree. Companion: [LINUX_QCOS.md](LINUX_QCOS.md) (the Linux side), [the staged EdTech architecture](../SAEM_EDTECH_ARCHITECTURE.md),
[the Work-Time Engine plan](WORKMATRIX.md), and [the daily intuition log](../../DAILY_INTUITION.md) (the day's reading, with the numbered equations cited below as "DI n").
Legend: **[V]** verified (we ran it, or read it in the project copy and wrote down where); **[P]** planned (a design no test has run yet).
Honest limit up front: no RV64 instruction of ours has been EXECUTED yet (no qemu or spike in this workspace). We compile, disassemble, link and
prove properties of the object code; the interpreter and the conformance digest are item #55 in `docs/STATUS.md`.

## 1. The topology and why

| Role | Core class | ISA and privilege | Runs | Why |
|---|---|---|---|---|
| "+1" monitor | SiFive E51 class | RV64IMAC, M and U only, no data cache | the sovereign kernel (`core/c17/`): gate, seal, journal, freeze | the one core that decides; small, in-order, no MMU, so little to attack and little to verify |
| "4" application | SiFive U54 class | RV64IMAFDC, M/S/U, Sv39, 32 KiB I$ and D$, coherent through a 2 MiB L2 | Linux in S-mode, the engines (forecast, mock test), the UI | speed and the ecosystem; never trusted for a decision |

[V] FU540-C000 manual (read 2026-10-31): hart 0 is the E51, harts 1 to 4 are U54s; the E51 has 16 KiB 2-way I-cache, 8 KiB DTIM, up to 8 KiB ITIM and no data cache.
Our "4+1" is the same shape as the FU540 on purpose: everything below can be tried on that part's documented behaviour before custom silicon exists.

Layers (the translation ladder runs top to bottom; the axiom MEANING != REPRESENTATION != EXECUTION != HARDWARE means each layer has its own verifier):

| Layer | What | Where it runs |
|---|---|---|
| L0 | RISC-V silicon: 1 monitor + 4 application harts | hardware |
| L1 | sovereign kernel, M-mode, `core/c17/` | monitor hart |
| THE DOOR | two shared-memory rings plus doorbells | between L1 and L2 |
| L2 | Linux, never trusted for decisions; `qcosd` sandbox | application harts |
| L3 | exact-math harness kernel (OpenMath role: `cos_rat`, SIR, an independent exact checker) | monitor hart or sandbox |
| L4 | engines (C++17 behind the JSON door) | application harts |
| L5 | app: Python orchestration and the HTML interface | application harts |

## 2. The kernel's two atomic profiles (built and proven 2026-11-01)

### 2.1 Why two

The monitor core cannot run the default profile. FU540 manual 3.5 (DI 10): on the E51 `lr` and `sc` raise an access exception on DTIM and every other uncached region;
AMOs work there, but the manual's errata ROCK-3 says the acquire/release ordering of E51 AMOs is wrong. The only atomic the E51 can be trusted with is a plain aligned load or store,
which RVWMO (unprivileged manual 17.1, DI 8) makes single-copy atomic. So `core/c17/cos_atomic.h` is now the ONLY file in the kernel that touches an atomic, and it has two profiles:

| | `COS_SINGLE_HART 0` (default) | `COS_SINGLE_HART 1` |
|---|---|---|
| For | many harts or threads call the kernel (the four application cores, the host tests, Phase 2 nodes) | exactly one hart ever runs kernel code: the monitor core; also a single-threaded embedding such as a phone app |
| Operation | the C11 atomic with the memory order written at each call site | relaxed load, compute, relaxed store |
| RV64 code | `lr.d/sc.d` or `amo*`, plus `fence` | no `lr`, `sc`, `amo`, `fence` at all |
| Memory layout | identical in both (17,840 bytes of caller memory in both; every shared word is `_Atomic uint32_t` or `_Atomic uint64_t`) | |
| Journal lock | `_Atomic uint32_t`, `amoswap.w.aq` spin | same word; a second entry from the same call stack is refused with `COS_E_BUSY` (new status code 21) instead of waited for |
| Threads in tests | yes, ThreadSanitizer | no; a re-entry test instead |

The contract of the one-hart profile (also in the header): one thread of control is inside the kernel at a time; an interrupt handler may set a flag the platform layer owns but never calls the kernel;
the door is not kernel state (it has its own module, may use `fence`, never `lr`/`sc`/`amo`).

### 2.2 What is proven, from the object code (`tools/check_riscv.py`, 13 checks, all [V])

1. Default profile unchanged: with the new header, eight of the nine objects (`cos_core.o`, `cos_pool.o` and six others) are byte-identical to the pre-change build (compared with `cmp`).
   Only `cos_journal.o` changed, as designed: the lock moved from an `atomic_flag` byte (which compiled to a word-sized `lr.w.aq/sc.w` loop, no byte AMO without Zabha) to one `amoswap.w.aq`.
   Atomic instructions in that object 27 to 18, `.text` 2816 to 2662 bytes.
2. One-hart profile: 9 objects, 5,218 instructions, zero `lr`/`sc`/`amo`/`fence`; the same detector finds 57 in the many-harts objects (so it is not blind). No float, no CSR, no trap, no `__atomic_*` import, no `.data/.bss`; it links.
   `.text` 15,414 bytes against 15,556.
3. Constrained loops (unprivileged manual 13.3, DI 6): every `lr..sc` loop in an audit image (all kernel code, no section garbage collection, branch targets resolved by the linker) is checked: at most 16 instructions,
   single entry, only base-I instructions between `lr` and `sc` (no load, store, fence, jump-and-link, multiply), no inner loop, `sc` has the size and address register of its `lr`, the retry branch follows the `sc`.
   Result: 7 loops (4 `lr.w`, 3 `lr.d`), longest 4 instructions, the exact shape of the manual's Listing 2 (`lr; bne; sc; bnez`). One failed-compare exit goes backwards to the caller's own retry code (the Treiber loop in `cos_pool_free`):
   that path reaches no `sc`, leaves the loop, and leaving is one of the events the manual's eventuality guarantee lists, so it is allowed and counted.
   Why it matters: "lock-free" on many harts only becomes the manual's livelock-freedom guarantee if the loops are constrained, and the platform's region has the eventuality property (the execution environment must say which regions do).
4. The checker is tested against itself: it must accept the canonical loop and reject 13 seeded violations (a load, store, fence, multiply or call between `lr` and `sc`; an inner backward branch; the address register overwritten; `lr.d` with `sc.w`; different address registers; 17 instructions; no retry branch; an `sc` with no `lr`; a jump into the middle).
5. `-mstrict-align` is pinned: the FU540 manual says misaligned accesses trap on both core types. With the flag every object is byte-identical to the build without it (clang already defaults to strict alignment here), while `-mno-strict-align` changes the code; the flag keeps it so if a toolchain default changes.

### 2.3 A finding worth keeping

`link.ld` discards `.riscv.attributes`. Without it `llvm-objdump` decodes a linked image as the base ISA only and prints every A, M and C instruction as `<unknown>`: a plain disassembly of our image showed
NO atomic instruction at all, which would have made any "the image contains lr/sc" check pass or fail for the wrong reason. The loop checker passes `--mattr=+m,+a,+c` and also asserts that the number of `lr` in the audit image
equals the number in the objects, and that the image has no `<unknown>`. Rule: never trust a disassembly of an image that lacks its ISA attribute.

### 2.4 Limits (stated, not hidden)

- 64-bit CAS needs RV64 (or a target with 64-bit lock-free atomics). On rv32 the default profile would call `__atomic_*_8`; the check would catch it. An rv32 profile is not planned.
- The one-hart profile has no protection against a violated contract except the journal re-entry detection. Pool and core counters are plain relaxed read-modify-write: an interrupt handler that called them would corrupt them.
- `-mstrict-align` only constrains the compiler; the hardware trap is the backstop.
- Zacas (`amocas`) and Zabha (byte AMOs) are not used. Zacas needs the platform memory attribute AMOCASW/D/Q (unprivileged 15.2); it could shorten the loops later.

## 3. Memory protection on the monitor core (PMP, Smepmp) [P unless marked]

Facts from the ratified privileged manual 3.7 and chapter 6 (DI 1 to 5) [V: read in the project copy]:
NAPOT region of 2^k bytes at base b: `pmpaddr = (b | (2^(k-1) - 1)) >> 2` (k >= 3). TOR entry i matches `pmpaddr[i-1] <= y < pmpaddr[i]`. The lowest-numbered entry that matches any byte decides and must match all bytes.
No match: M-mode succeeds, S and U fail. The L bit locks an entry (and `pmpaddr[i-1]` for TOR) until hart reset and enforces it on M-mode too. Granularity: write all ones to `pmpaddr0`, read back, G = lowest set bit, grain 2^(G+2).
Derived (not in the book): a locked NAPOT catch-all with permissions 000 at the highest index is default-deny for M-mode even without Smepmp.

The FU540 is the older v1.10 kind [V: manual]: 8 entries per hart, no `mseccfg`. Planned budget on a U54 (Linux side): three locked deny entries (E51 DTIM, E51 ITIM, the monitor's L2-LIM slice), the door window, and an allow-list for RAM and devices, all within 8 entries: tight.
Custom silicon should have 16 entries, Smepmp and an IOPMP.

Smepmp (`mseccfg` at CSR 0x747; MML bit 0 and MMWP bit 1 sticky; RLB bit 2 locked once cleared with any L set): with MML, L=1 means M-only and L=0 means S/U-only.
`LRWX = 0010` is a region M-mode may read and write and S/U may only read: a hardware one-way channel, our RESPONSE ring. `LRWX = 0011` is read/write for both: our REQUEST ring (the monitor treats it as hostile input anyway).
After MML is set an M-only executable rule cannot be added: set code regions before MML, or with RLB during boot.

DMA: the FU540's Ethernet GEM (64-bit scatter-gather) and PDMA are not checked by PMP and there is no IOMMU [V: manual]. A compromised Linux that can program them can write any physical memory, so on that part PMP alone is not a boundary.
Custom silicon needs an IOPMP: a PMP-like check at every bus master. This is the largest hardware gap in the plan.

## 4. The door (design only) [P]

Lessons from Linux `io_uring`, read in `/home/claude/torvalds/linux` (commit 36aad21d, 2026-10-30), and from the project's virtio-v1.1:

- Two single-producer single-consumer rings. Free-running `u32` head and tail, masked by `entries - 1`, power-of-two sizes; each side writes only ITS counter (`include/linux/io_uring_types.h:194` `struct io_rings`: "the kernel controls head of the sq ring and the tail of the cq ring, and the application controls tail of the sq ring and the head of the cq ring").
- Overflow is an explicit counter, not silent loss (`sq_dropped`, `cq_overflow` in the same struct). Our full-ring answer is an explicit status.
- The producer writes the slot, then store-releases the tail; the consumer load-acquires the tail, reads the slot, then store-releases the head. No read-modify-write is needed, so it runs on the E51 (`DAILY_INTUITION.md`, code patterns).
- Lifecycle "create disabled, register restrictions, then enable" (`io_uring/register.c:167` `io_register_restrictions`, `:277` `io_register_enable_rings`; the flag `IORING_SETUP_R_DISABLED` at `include/uapi/linux/io_uring.h:181` is checked with `smp_load_acquire` at `io_uring/io_uring.c:2869`).
- Shared memory is input, never state: the core keeps PRIVATE copies of head and tail and checks `(tail - head_private) <= capacity` on every read, because the other side can rewrite its counter at will. Linux itself documents that the application may modify pending completion entries (`io_uring_types.h`, comment on `cqes[]`).
- Slots: 64 bytes, `alignas(64)`, header `u16 kind, u16 flags, u32 len, u64 seq`, then an inline payload or an offset and length into a bulk area (never a raw pointer), explicit little-endian.
- Doorbells in the 4+1 topology: an `ecall` from a U54's S-mode traps to THAT U54's M-mode (OpenSBI), not to the E51, so the `ecall` door works only for single-hart configurations. In 4+1: CLINT `msip` at `0x0200_0000 + 4 * hart` toward the monitor, and the ChipLink MSI slave at `0x0202_0000` raising PLIC sources 32 to 41 toward Linux [V: FU540 memory map].
- Option for Phase 2: the same rings as a virtio transport (`include/uapi/linux/virtio_ring.h:104/111/128`: `vring_desc`, `vring_avail`, `vring_used`) so that a stock Linux driver can talk to a QCOS node.

## 5. FU540 facts the design leans on [V: manual, recorded 2026-10-31]

| Fact | Consequence for QCOS |
|---|---|
| ROCK-1: ITIM deallocation corrupts the I-cache | never deallocate the ITIM; link the monitor into L2-LIM |
| ROCK-2: the high 24 address bits are sometimes ignored | never rely on a trap for a wild pointer; validate every address arithmetically |
| ROCK-3: E51 AMO aq/rl ordering wrong | one-hart profile uses no AMO; if one is ever needed use `.aqrl` |
| ROCK-4: DPC not sign-extended; ROCK-5: watchpoints fire after the store | debug-only; note for any hardware-debug port |
| CCACHE-1: L2 uncorrectable-ECC address is unreliable | treat every uncorrectable ECC event as fatal: our FREEZE |
| L2-LIM: WayEnable only increases; FSBL enables 15 of 16 ways, leaving about 128 KiB of LIM | our image (15.7 KB code, 17.8 KB state) fits |
| Boot: reset vector ROM `0x1000/0x1004`, ZSBL mask ROM `0x1_0000` (hart 0 does the work), FSBL in L2-LIM `0x0800_0000`, BBL in DDR `0x8000_0000` (SBI plus payload) | the monitor must be the first code that runs on hart 0 and must lock PMP before releasing the other harts |
| OTP is 4096 x 32 bits, bits go 1 to 0 only; SiFive reserves the first 1 KiB | idea [P]: a unary monotonic epoch counter and a root-of-trust key hash; it is also the only rollback anchor the journal can have without an external device |
| Memory map: CLINT `0x0200_0000`, L2-LIM `0x0800_0000`, PLIC `0x0C00_0000` (53 sources, 7 priorities), PRCI `0x1000_0000`, UART0 `0x1001_0000`, DDR `0x8000_0000+` | link script and door addresses |

## 6. What this changes in the kernel's limits list

- The seal is a software HMAC (symmetric key). Planned: Ed25519 and a PMP lock of the sealed pages (Zknh gives SHA-256 instructions on silicon that has them, `zk.adoc`).
- Journal rollback to an older complete image is not detectable from the device alone; anchor `cos_journal_head()` outside (OTP counter idea above).
- The door, `qcosd`, the IOPMP and the OTP epoch are not written. The C# interop is not written.

## 7. Evidence index

- `riscv-privileged.pdf` (20250508): 3.7 PMP; chapter 6 Smepmp (local text copy used 2026-10-31).
- `riscv-unprivileged.pdf` (20250508): 13.2 Zalrsc and Listing 2; 13.3 constrained loops (lines 4167 to 4184 of the text copy); 15.2 Zacas; 16 Zabha; 17.1 RVWMO; appendix A.5 mappings.
- `FU540-C000-v1.0.pdf`: chapters 1, 3, 4, 5, 6, 9, 11, 12, 18, 25 (errata).
- `linus.pdf` (Torvalds, Helsinki 1997): 4.1 explicit sizes, byte order, alignment; 5.1 the Alpha port and byte-sized atomic flags.
- Tree: `core/c17/cos_atomic.h`, `core/c17/cos_journal.c`, `tools/check_riscv.py`, `tools/check_c17.py`, `core/tests/c17_{pool,journal,core}_test.c`.
