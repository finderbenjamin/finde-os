#include "panic.h"

#include "serial.h"

static void append_decimal(char* out, int value) {
  char tmp[16];
  int idx = 0;

  if (value == 0) {
    out[0] = '0';
    out[1] = '\0';
    return;
  }

  while (value > 0 && idx < (int)sizeof(tmp)) {
    tmp[idx++] = (char)('0' + (value % 10));
    value /= 10;
  }

  for (int i = 0; i < idx; ++i) {
    out[i] = tmp[idx - 1 - i];
  }
  out[idx] = '\0';
}

__attribute__((noreturn)) void panic(const char* msg) {
  serial_write("PANIC: ");
  serial_write(msg);
  serial_write("\n");

  for (;;) {
    __asm__ volatile ("cli; hlt");
  }
}

__attribute__((noreturn)) void panic_assert(const char* file, int line) {
  char line_buf[16];
  char message[128];
  int pos = 0;

  append_decimal(line_buf, line);

  for (const char* prefix = "ASSERT "; *prefix; ++prefix) {
    message[pos++] = *prefix;
  }

  while (*file && pos < (int)sizeof(message) - 1) {
    message[pos++] = *file++;
  }

  if (pos < (int)sizeof(message) - 1) {
    message[pos++] = ':';
  }

  for (const char* p = line_buf; *p && pos < (int)sizeof(message) - 1; ++p) {
    message[pos++] = *p;
  }

  message[pos] = '\0';
  panic(message);
}
