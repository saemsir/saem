# Bare-metal entry for the sovereign kernel image: physical addressing, no MMU, no interrupts. One hart runs; the others (if any) park.
# Contract: the image is loaded at the RAM base (0x80000000). a0 = hart id as in the RISC-V boot convention.
        .section .text.start, "ax", @progbits
        .globl _start
_start:
        .option push
        .option norelax
        la      gp, __global_pointer$
        .option pop
        bnez    a0, park                 # only hart 0 runs; any other hart parks (the monitor core is hart 0 on the FU540)
        la      sp, __stack_top
        call    qcos_main
park:
        wfi
        j       park
