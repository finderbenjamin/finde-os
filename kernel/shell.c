#include "shell.h"

#include <stddef.h>
#include <stdint.h>

#include "console.h"
#include "heap.h"
#include "idt.h"

#define SHELL_MAX_LINE 128

static char g_line[SHELL_MAX_LINE + 1];
static size_t g_cursor = 0;

static void write_u64(uint64_t value) {
  char buf[21];
  size_t i = 0;

  if (value == 0) {
    console_write("0");
    return;
  }

  while (value > 0 && i < sizeof(buf)) {
    buf[i++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while (i > 0) {
    console_write_char(buf[--i]);
  }
}

static void write_hex_u64(uint64_t value) {
  static const char* digits = "0123456789ABCDEF";

  console_write("0x");
  for (int shift = 60; shift >= 0; shift -= 4) {
    console_write_char(digits[(value >> shift) & 0xF]);
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
  console_write("finde-os> ");
}

static void cmd_help(void) {
  console_write("commands: help ticks malloc\n");
}

static void cmd_ticks(void) {
  console_write("ticks: ");
  write_u64(timer_ticks_get());
  console_write("\n");
}

static void cmd_malloc(void) {
  void* a = kmalloc(16);
  void* b = kmalloc(32);
  void* c = kmalloc(64);

  console_write("malloc: ");
  write_hex_u64((uint64_t)(uintptr_t)a);
  console_write(" ");
  write_hex_u64((uint64_t)(uintptr_t)b);
  console_write(" ");
  write_hex_u64((uint64_t)(uintptr_t)c);
  console_write("\n");
}

static void shell_execute_line(void) {
  char command[SHELL_MAX_LINE + 1];
  size_t i = 0;

  while (i < g_cursor && g_line[i] == ' ') {
    ++i;
  }

  size_t j = 0;
  while (i < g_cursor && g_line[i] != ' ' && j < SHELL_MAX_LINE) {
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
    console_write("unknown\n");
  }
}

void shell_init_minimal(void) {
  g_cursor = 0;
}

void shell_init(void) {
  shell_init_minimal();
  console_write("finde-os shell\n");
  shell_prompt();
}

void shell_process_input_char_for_test(char c) {
  if (c == '\r' || c == '\n') {
    console_write("\n");
    g_line[g_cursor] = '\0';
    shell_execute_line();
    g_cursor = 0;
    shell_prompt();
    return;
  }

  if (c == 0x7F || c == '\b') {
    if (g_cursor > 0) {
      --g_cursor;
      g_line[g_cursor] = '\0';
      console_write("\b \b");
    }
    return;
  }

  if (c >= 32 && c <= 126) {
    if (g_cursor < SHELL_MAX_LINE) {
      g_line[g_cursor++] = c;
      g_line[g_cursor] = '\0';
      console_write_char(c);
    }
  }
}

const char* shell_current_line_for_test(void) {
  g_line[g_cursor] = '\0';
  return g_line;
}

size_t shell_cursor_for_test(void) {
  return g_cursor;
}

void shell_step(void) {
  if (!console_has_char()) {
    return;
  }

  shell_process_input_char_for_test(console_read_char());
}
