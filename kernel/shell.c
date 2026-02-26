#include "shell.h"

#include <stddef.h>
#include <stdint.h>

#include "heap.h"
#include "idt.h"
#include "serial.h"

#define SHELL_MAX_LINE 80

static char g_line[SHELL_MAX_LINE + 1];
static size_t g_line_len = 0;

static void write_u64(uint64_t value) {
  char buf[21];
  size_t i = 0;

  if (value == 0) {
    serial_write("0");
    return;
  }

  while (value > 0 && i < sizeof(buf)) {
    buf[i++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while (i > 0) {
    serial_write_char(buf[--i]);
  }
}

static void write_hex_u64(uint64_t value) {
  static const char* digits = "0123456789ABCDEF";

  serial_write("0x");
  for (int shift = 60; shift >= 0; shift -= 4) {
    serial_write_char(digits[(value >> shift) & 0xF]);
  }
}

static int streq(const char* a, const char* b) {
  while (*a && *b) {
    if (*a != *b) {
      return 0;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

static void shell_prompt(void) {
  serial_write("> ");
}

static void cmd_help(void) {
  serial_write("commands: help ticks malloc\n");
}

static void cmd_ticks(void) {
  serial_write("ticks: ");
  write_u64(timer_ticks_get());
  serial_write("\n");
}

static void cmd_malloc(void) {
  void* a = kmalloc(16);
  void* b = kmalloc(32);
  void* c = kmalloc(64);

  serial_write("malloc: ");
  write_hex_u64((uint64_t)(uintptr_t)a);
  serial_write(" ");
  write_hex_u64((uint64_t)(uintptr_t)b);
  serial_write(" ");
  write_hex_u64((uint64_t)(uintptr_t)c);
  serial_write("\n");
}

static void shell_execute_line(void) {
  char command[SHELL_MAX_LINE + 1];
  size_t i = 0;

  while (i < g_line_len && g_line[i] == ' ') {
    ++i;
  }

  size_t j = 0;
  while (i < g_line_len && g_line[i] != ' ' && j < SHELL_MAX_LINE) {
    command[j++] = g_line[i++];
  }
  command[j] = '\0';

  if (command[0] == '\0') {
    return;
  }

  if (streq(command, "help")) {
    cmd_help();
  } else if (streq(command, "ticks")) {
    cmd_ticks();
  } else if (streq(command, "malloc")) {
    cmd_malloc();
  } else {
    serial_write("unknown\n");
  }
}

void shell_init_minimal(void) {
  g_line_len = 0;
}

void shell_init(void) {
  shell_init_minimal();
  serial_write("finde shell\n");
  shell_prompt();
}

void shell_step(void) {
  if (!serial_received()) {
    return;
  }

  char c = serial_read_char();

  if (c == '\r' || c == '\n') {
    serial_write("\n");
    g_line[g_line_len] = '\0';
    shell_execute_line();
    g_line_len = 0;
    shell_prompt();
    return;
  }

  if (c == 0x7F || c == '\b') {
    if (g_line_len > 0) {
      --g_line_len;
      serial_write("\b \b");
    }
    return;
  }

  if (c >= 32 && c <= 126) {
    if (g_line_len < SHELL_MAX_LINE) {
      g_line[g_line_len++] = c;
      serial_write_char(c);
    }
  }
}
