/* Memory map of a monitor-core image: RAM at 0x80000000 (as on the SiFive FU540 and on QEMU virt), code first, then read-only data, then
 * the stack and the caller-provided kernel memory. No heap, no .data/.bss in the kernel objects; the runtime stubs own the few bytes below. */
OUTPUT_ARCH(riscv)
ENTRY(_start)
MEMORY { RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 4M }
SECTIONS {
  .text : { *(.text.start) *(.text .text.*) } > RAM
  .rodata : { *(.rodata .rodata.* .srodata .srodata.*) } > RAM
  .data : { *(.data .data.* .sdata .sdata.*) } > RAM
  __global_pointer$ = . + 0x800;
  .bss (NOLOAD) : { *(.sbss .sbss.* .bss .bss.* COMMON) . = ALIGN(64); } > RAM
  . = ALIGN(64);
  __stack_bottom = .;
  . = . + 64K;
  __stack_top = .;
  /DISCARD/ : { *(.comment) *(.note*) *(.riscv.attributes) }
}
