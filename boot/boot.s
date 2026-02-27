.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0xE85250D6
.set ARCH,     0
.set HEADER_LEN, (header_end - header_start)
.set CHECKSUM, -(MAGIC + ARCH + HEADER_LEN)

.set CR0_PE,    0x00000001
.set CR0_PG,    0x80000000
.set CR4_PAE,   0x00000020
.set EFER_MSR,  0xC0000080
.set EFER_LME,  0x00000100

.set GDT64_CODE, 0x08
.set GDT64_DATA, 0x10

.section .multiboot, "a"
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
boot32_stack_bottom:
  .skip 4096
boot32_stack_top:

.align 16
stack_bottom:
  .skip 16384
stack_top:

.align 4096
pml4_table:
  .skip 4096

.align 4096
pdpt_table:
  .skip 4096

.align 4096
pd_tables:
  .skip 16384

.align 8
mb_magic64:
  .quad 0
mb_info64:
  .quad 0

.section .rodata
.align 8
gdt64:
  .quad 0x0000000000000000
  .quad 0x00AF9A000000FFFF
  .quad 0x00AF92000000FFFF
gdt64_end:

gdt64_descriptor:
  .word gdt64_end - gdt64 - 1
  .quad gdt64

.section .text
.global _start
.extern kernel_main

.code32
_start:
  cli

  mov $boot32_stack_top, %esp
  mov %esp, %ebp

  mov %eax, mb_magic64
  mov %ebx, mb_info64

  movl $0, pml4_table
  movl $0, pml4_table + 4
  movl $0, pdpt_table
  movl $0, pdpt_table + 4

  movl $pdpt_table, %eax
  orl $0x3, %eax
  movl %eax, pml4_table
  movl $0, pml4_table + 4

  xorl %ecx, %ecx
map_pdpt_entries:
  movl $pd_tables, %eax
  movl %ecx, %edx
  shll $12, %edx
  addl %edx, %eax
  orl $0x3, %eax
  movl %eax, pdpt_table(,%ecx,8)
  movl $0, pdpt_table + 4(,%ecx,8)
  incl %ecx
  cmpl $4, %ecx
  jne map_pdpt_entries

  xorl %ecx, %ecx
map_pd_entries:
  movl %ecx, %eax
  shll $21, %eax
  orl $0x83, %eax
  movl %eax, pd_tables(,%ecx,8)
  movl $0, pd_tables + 4(,%ecx,8)
  incl %ecx
  cmpl $2048, %ecx
  jne map_pd_entries

  movl %cr4, %eax
  orl $CR4_PAE, %eax
  movl %eax, %cr4

  movl $pml4_table, %eax
  movl %eax, %cr3

  movl $EFER_MSR, %ecx
  rdmsr
  orl $EFER_LME, %eax
  wrmsr

  movl %cr0, %eax
  orl $(CR0_PE | CR0_PG), %eax
  movl %eax, %cr0

  lgdt gdt64_descriptor
  ljmp $GDT64_CODE, $long_mode_entry

.code64
long_mode_entry:
  mov $GDT64_DATA, %ax
  mov %ax, %ds
  mov %ax, %es
  mov %ax, %ss

  mov $stack_top, %rsp
  mov %rsp, %rbp

  mov mb_magic64(%rip), %rdi
  mov mb_info64(%rip), %rsi

  call kernel_main

hang:
  cli
  hlt
  jmp hang
