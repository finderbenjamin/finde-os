#include "cli_executor.h"

#include <stddef.h>

#include "cap.h"
#include "console.h"
#include "heap.h"
#include "idt.h"

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

static uint32_t op_to_rights(const char* operation) {
  if (operation == 0) {
    return 0u;
  }

  if (operation[0] == 'r' && operation[1] == 'e' && operation[2] == 'a' && operation[3] == 'd' && operation[4] == '\0') {
    return CAP_R_READ;
  }

  if (operation[0] == 'w' && operation[1] == 'r' && operation[2] == 'i' && operation[3] == 't' && operation[4] == 'e' && operation[5] == '\0') {
    return CAP_R_WRITE;
  }

  if (operation[0] == 'e' && operation[1] == 'x' && operation[2] == 'e' && operation[3] == 'c' && operation[4] == '\0') {
    return CAP_R_EXEC;
  }

  return 0u;
}

void cli_execute_validated(const cli_validated_command_t* cmd) {
  if (cmd == 0 || cmd->status != CLI_VALIDATE_OK) {
    return;
  }

  if (cmd->ast.kind == CLI_AST_EMPTY) {
    return;
  }

  if (cmd->ast.kind == CLI_AST_HELP) {
    console_write("commands: help status ticks malloc cap list|show|check\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_STATUS) {
    console_write("STATUS mode=");
    if (cmd->mode == CLI_MODE_MICROVM) {
      console_write("microvm");
    } else {
      console_write("sandbox");
    }
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_TICKS) {
    console_write("ticks: ");
    write_u64(timer_ticks_get());
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_MALLOC) {
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
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_LIST) {
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
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_SHOW) {
    cap_snapshot_t snap;
    if (cap_audit(cmd->handle, &snap.type, &snap.rights, &snap.generation) == 0) {
      return;
    }

    console_write("CAP_SHOW id=");
    write_u64(cmd->handle);
    console_write(" type=");
    write_u64(snap.type);
    console_write(" rights=");
    write_rights(snap.rights);
    console_write(" generation=");
    write_u64(snap.generation);
    console_write("\n");
    return;
  }

  if (cmd->ast.kind == CLI_AST_CAP_CHECK) {
    uint32_t rights = op_to_rights(cmd->ast.arg0);
    console_write("CAP_CHECK op=");
    console_write(cmd->ast.arg0);
    if (rights == 0u || cmd->handle == 0) {
      console_write(" result=DENY reason=");
      console_write(cap_deny_reason());
      console_write("\n");
      return;
    }

    console_write(" result=ALLOW id=");
    write_u64(cmd->handle);
    console_write("\n");
  }
}
