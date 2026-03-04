#include "shell.h"

#include <stddef.h>
#include <stdint.h>

#include "cap.h"
#include "console.h"
#include "heap.h"
#include "idt.h"

#define SHELL_MAX_LINE 128
#define SHELL_MAX_TOKENS 8

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

static int parse_u64(const char* s, uint64_t* value_out) {
  uint64_t value = 0;
  if (s == 0 || *s == '\0') {
    return 0;
  }

  while (*s != '\0') {
    if (*s < '0' || *s > '9') {
      return 0;
    }
    value = (value * 10u) + (uint64_t)(*s - '0');
    ++s;
  }

  *value_out = value;
  return 1;
}

static void write_rights(uint32_t rights) {
  if (rights == 0u) {
    console_write("none");
    return;
  }

  int wrote = 0;
  if ((rights & CAP_R_READ) != 0u) {
    console_write("read");
    wrote = 1;
  }
  if ((rights & CAP_R_WRITE) != 0u) {
    if (wrote) {
      console_write("|");
    }
    console_write("write");
    wrote = 1;
  }
  if ((rights & CAP_R_EXEC) != 0u) {
    if (wrote) {
      console_write("|");
    }
    console_write("exec");
  }
}

static void shell_prompt(void) {
  console_write("finde-os> ");
}

static void cmd_help(void) {
  console_write("commands: help ticks malloc cap list|show|check\n");
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

static uint32_t op_to_rights(const char* operation) {
  if (streq(operation, "read")) {
    return CAP_R_READ;
  }
  if (streq(operation, "write")) {
    return CAP_R_WRITE;
  }
  if (streq(operation, "exec")) {
    return CAP_R_EXEC;
  }
  return 0u;
}

static void cmd_cap_list(void) {
  cap_snapshot_t snap;
  int found = 0;
  for (uint32_t i = 0; i < cap_capacity(); ++i) {
    if (cap_snapshot_at(i, &snap) == 0) {
      continue;
    }

    found = 1;
    console_write("CAP_LIST id=");
    write_u64(snap.handle);
    console_write(" type=");
    write_u64(snap.type);
    console_write(" rights=");
    write_rights(snap.rights);
    console_write("\n");
  }

  if (!found) {
    console_write("CAP_LIST empty\n");
  }
}

static void cmd_cap_show(const char* id_token) {
  uint64_t handle;
  cap_snapshot_t snap;
  if (!parse_u64(id_token, &handle) || cap_audit(handle, &snap.type, &snap.rights, &snap.generation) == 0) {
    console_write("CAP_SHOW deny=");
    console_write(cap_deny_reason());
    console_write("\n");
    return;
  }

  console_write("CAP_SHOW id=");
  write_u64(handle);
  console_write(" type=");
  write_u64(snap.type);
  console_write(" rights=");
  write_rights(snap.rights);
  console_write(" generation=");
  write_u64(snap.generation);
  console_write("\n");
}

static void cmd_cap_check(const char* operation) {
  uint32_t rights = op_to_rights(operation);
  const char* deny_reason = 0;

  if (rights == 0u) {
    console_write("CAP_CHECK deny=");
    console_write(cap_deny_reason());
    console_write("\n");
    return;
  }

  for (uint32_t i = 0; i < cap_capacity(); ++i) {
    cap_snapshot_t snap;
    if (cap_snapshot_at(i, &snap) == 0) {
      continue;
    }

    if (cap_require(snap.handle, snap.type, rights, &deny_reason) == CAP_REQUIRE_OK) {
      console_write("CAP_CHECK op=");
      console_write(operation);
      console_write(" result=ALLOW id=");
      write_u64(snap.handle);
      console_write("\n");
      return;
    }
  }

  console_write("CAP_CHECK op=");
  console_write(operation);
  console_write(" result=DENY reason=");
  console_write(cap_deny_reason());
  console_write("\n");
}

static void shell_execute_tokens(char* tokens[], size_t count) {
  if (count == 0) {
    return;
  }

  if (streq(tokens[0], "help")) {
    cmd_help();
    return;
  }
  if (streq(tokens[0], "ticks")) {
    cmd_ticks();
    return;
  }
  if (streq(tokens[0], "malloc")) {
    cmd_malloc();
    return;
  }

  if (streq(tokens[0], "cap")) {
    if (count >= 2 && streq(tokens[1], "list")) {
      cmd_cap_list();
      return;
    }
    if (count >= 3 && streq(tokens[1], "show")) {
      cmd_cap_show(tokens[2]);
      return;
    }
    if (count >= 3 && streq(tokens[1], "check")) {
      cmd_cap_check(tokens[2]);
      return;
    }
  }

  console_write("unknown\n");
}

void shell_execute_line_for_test(const char* line) {
  char scratch[SHELL_MAX_LINE + 1];
  char* tokens[SHELL_MAX_TOKENS];
  size_t count = 0;
  size_t i = 0;

  while (line[i] != '\0' && i < SHELL_MAX_LINE) {
    scratch[i] = line[i];
    ++i;
  }
  scratch[i] = '\0';

  i = 0;
  while (scratch[i] != '\0' && count < SHELL_MAX_TOKENS) {
    while (scratch[i] == ' ') {
      ++i;
    }
    if (scratch[i] == '\0') {
      break;
    }

    tokens[count++] = &scratch[i];
    while (scratch[i] != '\0' && scratch[i] != ' ') {
      ++i;
    }
    if (scratch[i] == '\0') {
      break;
    }
    scratch[i] = '\0';
    ++i;
  }

  shell_execute_tokens(tokens, count);
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
    shell_execute_line_for_test(g_line);
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
