#include "console.h"

#include "keyboard.h"
#include "serial.h"
#include "vga.h"

void console_init(void) {
  serial_init();
#ifndef KEYBOARD_TEST
  vga_init();
#endif
}

void console_write_char(char c) {
#ifndef KEYBOARD_TEST
  vga_putc(c);
#endif
  serial_write_char(c);
}

void console_write(const char* s) {
  while (*s) {
    console_write_char(*s++);
  }
}

int console_has_char(void) {
  return keyboard_has_char() || serial_received();
}

char console_read_char(void) {
  if (keyboard_has_char()) {
    return keyboard_read_char();
  }

  return serial_read_char();
}
