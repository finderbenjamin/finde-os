#include <stddef.h>
#include <stdint.h>

#include "heap.h"
#include "idt.h"
#include "panic.h"
#include "serial.h"

extern void isr_4(void);

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

void kernel_main(void) {
#ifdef HEAP_TEST
  __asm__ volatile ("cli");
  heap_test();
  serial_write("HEAP_OK\n");
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
  serial_write("BOOT_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
