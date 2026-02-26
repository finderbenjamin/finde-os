.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0xE85250D6
.set ARCH,     0
.set HEADER_LEN, (header_end - header_start)
.set CHECKSUM, -(MAGIC + ARCH + HEADER_LEN)

.section .multiboot
.align 8
header_start:
  .long MAGIC
  .long ARCH
  .long HEADER_LEN
  .long CHECKSUM

  /* end tag */
  .short 0
  .short 0
  .long 8
header_end:

.section .bss
.align 16
stack_bottom:
  .skip 16384
stack_top:

.section .text
.global _start
.extern kernel_main

_start:
  cli
  mov $stack_top, %rsp
  mov %rsp, %rbp
  mov %rax, %rdi
  mov %rbx, %rsi
  call kernel_main

hang:
  hlt
  jmp hang
