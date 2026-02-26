.macro ISR_NOERR n
.global isr_\n
isr_\n:
  pushq $0
  pushq $\n
  jmp isr_common
.endm

.macro ISR_ERR n
.global isr_\n
isr_\n:
  pushq $\n
  jmp isr_common
.endm

.section .text
.code64

.extern g_timer_ticks
.extern keyboard_handle_scancode

.set COM1, 0x3F8

serial_write_al:
  pushq %rdx
  pushq %rcx
  movb %al, %cl
1:
  movw $(COM1 + 5), %dx
  inb %dx, %al
  testb $0x20, %al
  jz 1b
  movw $COM1, %dx
  movb %cl, %al
  outb %al, %dx
  popq %rcx
  popq %rdx
  ret

isr_common:
  cli
  movq (%rsp), %rbx

  movb $'E', %al
  call serial_write_al
  movb $'X', %al
  call serial_write_al
  movb $'C', %al
  call serial_write_al
  movb $':', %al
  call serial_write_al

  movq %rbx, %rax
  cmpq $10, %rax
  jb 2f

  xorq %rdx, %rdx
  movq $10, %rcx
  divq %rcx

  addb $'0', %al
  call serial_write_al
  movq %rdx, %rax

2:
  addb $'0', %al
  call serial_write_al
  movb $'\n', %al
  call serial_write_al

3:
  hlt
  jmp 3b

.global irq_0
irq_0:
  pushq %rax
  pushq %rdx
  incq g_timer_ticks(%rip)
  movb $0x20, %al
  movw $0x20, %dx
  outb %al, %dx
  popq %rdx
  popq %rax
  iretq

.global irq_1
irq_1:
  pushq %rax
  pushq %rdx
  pushq %rdi
  subq $8, %rsp
  movw $0x60, %dx
  inb %dx, %al
  movzbq %al, %rdi
  call keyboard_handle_scancode
  addq $8, %rsp
  movb $0x20, %al
  movw $0x20, %dx
  outb %al, %dx
  popq %rdi
  popq %rdx
  popq %rax
  iretq

.global irq_ignore
irq_ignore:
  pushq %rax
  pushq %rdx
  movb $0x20, %al
  movw $0xA0, %dx
  outb %al, %dx
  movw $0x20, %dx
  outb %al, %dx
  popq %rdx
  popq %rax
  iretq

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR 29
ISR_ERR 30
ISR_NOERR 31
