#include <stdint.h>

#include "serial.h"

#define COM1 0x3F8

static int g_initialized = 0;

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void serial_init(void) {
  if (g_initialized) {
    return;
  }

  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 0x03);
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x03);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);

  g_initialized = 1;
}

static int serial_tx_empty(void) {
  return (inb(COM1 + 5) & 0x20) != 0;
}

int serial_received(void) {
  serial_init();
  return (inb(COM1 + 5) & 0x01) != 0;
}

char serial_read_char(void) {
  serial_init();
  while (!serial_received()) {
  }
  return (char)inb(COM1);
}

void serial_write_char(char c) {
  serial_init();
  while (!serial_tx_empty()) {
  }
  outb(COM1, (uint8_t)c);
}

void serial_write(const char* s) {
  serial_init();

  while (*s) {
    serial_write_char(*s++);
  }
}
