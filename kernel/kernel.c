#include "panic.h"
#include "serial.h"

void kernel_main(void) {
#ifdef PANIC_TEST
  panic("TEST");
#endif

  ASSERT(1);
  serial_write("BOOT_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
