ENTRY(_start)

SECTIONS {
  . = 1M;

  .text : {
    KEEP(*(.multiboot))
    *(.text*)
  }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : {
    *(.bss*)
    *(COMMON)
    . = ALIGN(16);
    stack_bottom = .;
    . += 16384;
    stack_top = .;
  }
}
