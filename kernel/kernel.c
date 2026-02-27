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

volatile uint8_t g_nx_expect = 0;

#if defined(PMM_TEST) || defined(VMM_TEST) || defined(NX_TEST)
static uint64_t align_down_4k(uint64_t value) {
  return value & ~0xFFFull;
}

static uint64_t align_up_4k(uint64_t value) {
  return (value + 0xFFFull) & ~0xFFFull;
}
#endif

#ifdef PMM_TEST
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



#ifdef PF_TEST
static __attribute__((noreturn)) void pf_test_fail(void) {
  serial_write("PF_FAIL\n");
  panic("PF_TEST");
}
#endif

#ifdef VMM_TEST
static __attribute__((noreturn)) void vmm_test_fail(void) {
  serial_write("VMM_FAIL\n");
  panic("VMM_TEST");
}

static __attribute__((noreturn)) void vmm_test_halt_success(void) {
  serial_write("VMM_OK\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
#endif


#ifdef NX_TEST
static __attribute__((noreturn)) void nx_test_fail(void) {
  serial_write("NX_FAIL\n");
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

  pmm_test_halt_success();
#endif

#ifdef VMM_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    vmm_test_fail();
  }

  const uint64_t mb_info_phys = (uint64_t)(uint32_t)mb_info_addr;
  pmm_reserve_range(0, 0x2000000ull);

  const uint64_t kernel_start = align_down_4k((uint64_t)(uintptr_t)&_kernel_start);
  const uint64_t kernel_end = align_up_4k((uint64_t)(uintptr_t)&_kernel_end);
  pmm_reserve_range(kernel_start, kernel_end);

  const uint64_t mb_start = align_down_4k(mb_info_phys);
  const uint64_t mb_end = align_up_4k(mb_info_phys + 4096ull);
  pmm_reserve_range(mb_start, mb_end);

  const uint64_t data_page = pmm_alloc_page();
  if (data_page == 0) {
    vmm_test_fail();
  }

  const uint64_t test_virt = data_page;
  if (vmm_map_page(test_virt, data_page, VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE) != 0) {
    pmm_free_page(data_page);
    vmm_test_fail();
  }

  volatile uint64_t* test_ptr = (volatile uint64_t*)(uintptr_t)test_virt;
  const uint64_t pattern = 0x1122334455667788ull;
  *test_ptr = pattern;
  if (*test_ptr != pattern) {
    pmm_free_page(data_page);
    vmm_test_fail();
  }

  pmm_free_page(data_page);
  vmm_test_halt_success();
#endif


#ifdef NX_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    nx_test_fail();
  }

  idt_init();
  interrupts_init();

  uint64_t efer_low;
  uint64_t efer_high;
  __asm__ volatile ("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));
  uint64_t efer = (efer_high << 32) | efer_low;
  efer |= (1ull << 11);
  __asm__ volatile ("wrmsr" : : "c"(0xC0000080), "a"((uint32_t)efer), "d"((uint32_t)(efer >> 32)) : "memory");

  const uint64_t mb_info_phys = (uint64_t)(uint32_t)mb_info_addr;
  pmm_reserve_range(0, 0x2000000ull);

  const uint64_t kernel_start = align_down_4k((uint64_t)(uintptr_t)&_kernel_start);
  const uint64_t kernel_end = align_up_4k((uint64_t)(uintptr_t)&_kernel_end);
  pmm_reserve_range(kernel_start, kernel_end);

  const uint64_t mb_start = align_down_4k(mb_info_phys);
  const uint64_t mb_end = align_up_4k(mb_info_phys + 4096ull);
  pmm_reserve_range(mb_start, mb_end);

  const uint64_t code_page = pmm_alloc_page();
  if (code_page == 0) {
    nx_test_fail();
  }

  uint8_t* code_ptr = (uint8_t*)(uintptr_t)code_page;
  code_ptr[0] = 0xC3;
  code_ptr[1] = 0x90;
  code_ptr[2] = 0x90;
  code_ptr[3] = 0x90;

  const uint64_t test_virt = 0x0000000100000000ull;
  if (vmm_map_page(test_virt, code_page, VMM_PAGE_PRESENT | VMM_PAGE_WRITABLE | VMM_PAGE_NO_EXECUTE) != 0) {
    nx_test_fail();
  }

  g_nx_expect = 1;
  const uint64_t nx_target = test_virt;
  __asm__ volatile ("mov %0, %%rax; call *%%rax" : : "r"(nx_target) : "rax", "memory");

  nx_test_fail();
#endif


#ifdef PF_TEST
  serial_init();
  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    pf_test_fail();
  }

  idt_init();
  interrupts_init();

  *(volatile uint64_t*)(uintptr_t)0x00000DEADBEEF000ull = 0x1;

  serial_write("PF_FAIL\n");
  __asm__ volatile ("cli");
  for (;;) {
    __asm__ volatile ("hlt");
  }
#endif


#ifdef VGA_TEST
  __asm__ volatile ("cli");
  serial_init();
  vga_init();
  vga_write("VGA WORKS\n");
  serial_write("VGA_OK\n");
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
