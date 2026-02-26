#include "idt.h"
#include "panic.h"
#include "serial.h"

extern void isr_4(void);

void kernel_main(void) {
  idt_init();

#ifdef IDT_TEST
  isr_4();
#endif

#ifdef PANIC_TEST
  panic("TEST");
#endif

  ASSERT(1);
  serial_write("BOOT_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
