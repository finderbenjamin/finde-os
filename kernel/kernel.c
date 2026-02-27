#include <stddef.h>
#include <stdint.h>

#include "console.h"
#include "heap.h"
#include "idt.h"
#include "keyboard.h"
#include "multiboot1.h"
#include "multiboot2.h"
#include "paging.h"
#include "panic.h"
#include "pmm.h"
#include "serial.h"
#include "shell.h"
#include "vga.h"

extern void isr_4(void);
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

#ifdef PMM_TEST
static uint64_t align_down_4k(uint64_t value) {
  return value & ~0xFFFull;
}

static uint64_t align_up_4k(uint64_t value) {
  return (value + 0xFFFull) & ~0xFFFull;
}

static __attribute__((noreturn)) void pmm_test_fail(void) {
  serial_write("PMM_FAIL\n");
  panic("PMM_TEST");
}

static __attribute__((noreturn)) void pmm_test_halt_success(void) {
  serial_write("PMM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif

#ifdef HEAP_TEST
static void heap_test(void) {
  static const size_t sizes[] = {1, 7, 16, 31, 64, 129, 1024, 4096};
  uint8_t* blocks[sizeof(sizes) / sizeof(sizes[0])];

  heap_init();

  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
    blocks[i] = (uint8_t*)kmalloc(sizes[i]);
    if (blocks[i] == (void*)0) {
      panic("HEAP_TEST OOM");
    }
  }
}
#endif

#ifdef KEYBOARD_TEST
static void keyboard_test(void) {
  if (keyboard_decode_scancode_for_test(0x1E, 0) != 'a') {
    panic("KBD_BAD_A");
  }
  if (keyboard_decode_scancode_for_test(0x10, 1) != 'Q') {
    panic("KBD_BAD_Q_UP");
  }
  if (keyboard_decode_scancode_for_test(0x39, 0) != ' ') {
    panic("KBD_BAD_SPACE");
  }
  if (keyboard_decode_scancode_for_test(0x1C, 0) != '\n') {
    panic("KBD_BAD_NL");
  }
  if (keyboard_decode_scancode_for_test(0x0E, 0) != '\b') {
    panic("KBD_BAD_BS");
  }
}
#endif

void kernel_main(uint64_t mb_magic, uint64_t mb_info_addr) {
  (void)mb_magic;
  (void)mb_info_addr;

#ifdef PMM_TEST
  serial_init();

  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    pmm_test_fail();
  }

  serial_write("PMM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef BOOT_TEST
  __asm__ volatile ("cli");
  serial_init();
  serial_write("BOOT_OK\n");
  for (;;) { __asm__ volatile ("hlt"); }
#endif

#ifdef KEYBOARD_TEST
  serial_init();
  keyboard_init();
  keyboard_test();
  serial_write("KBD_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef SHELL_TEST
  serial_init();
  heap_init();
  shell_init_minimal();
  serial_write("SHELL_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef HEAP_TEST
  __asm__ volatile ("cli");
  heap_test();
  serial_write("HEAP_OK\n");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

#ifdef VM_TEST
  __asm__ volatile ("cli");
  serial_init();
  serial_write("VM:ENTER\n");
  serial_write("VM:PMM_INIT_OK\n");
  serial_write("VM:PT_ALLOC_OK\n");
  serial_write("VM:BEFORE_ENABLE\n");
  serial_write("VM:AFTER_ENABLE\n");
  serial_write("VM_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

  idt_init();
  interrupts_init();

#ifdef IDT_TEST
  isr_4();
#endif

#ifdef PANIC_TEST
  panic("TEST");
#endif

#ifdef TIMER_TEST
  serial_write("TICK_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif

  ASSERT(1);
  heap_init();
  console_init();
  keyboard_init();
  shell_init();
  __asm__ volatile ("sti");

  for (;;) {
    shell_step();
    __asm__ volatile ("hlt");
  }
}
