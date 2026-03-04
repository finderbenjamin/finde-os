#include "cli_validator.h"

#include "cap.h"

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

int cli_validate_ast(const cli_ast_t* ast, cli_mode_t mode, cli_validated_command_t* out) {
  if (ast == 0 || out == 0) {
    return 0;
  }

  out->ast = *ast;
  out->status = CLI_VALIDATE_OK;
  out->reason = "OK";
  out->handle = 0;

  if (ast->kind == CLI_AST_EMPTY) {
    return 1;
  }

  if (ast->kind == CLI_AST_UNKNOWN) {
    out->status = CLI_VALIDATE_SYNTAX;
    out->reason = "unknown command";
    return 1;
  }

  if (ast->kind == CLI_AST_MALLOC && mode == CLI_MODE_MICROVM) {
    out->status = CLI_VALIDATE_DENY;
    out->reason = "DENY: command not allowed in microvm mode";
    return 1;
  }

  if (ast->kind == CLI_AST_CAP_SHOW) {
    uint64_t handle;
    if (!parse_u64(ast->arg0, &handle)) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "cap show requires numeric id";
      return 1;
    }

    if (cap_audit(handle, 0, 0, 0) == 0) {
      out->status = CLI_VALIDATE_DENY;
      out->reason = cap_deny_reason();
      return 1;
    }

    out->handle = handle;
    return 1;
  }

  if (ast->kind == CLI_AST_CAP_CHECK) {
    uint32_t rights = op_to_rights(ast->arg0);
    if (rights == 0u) {
      out->status = CLI_VALIDATE_DENY;
      out->reason = cap_deny_reason();
      return 1;
    }

    for (uint32_t i = 0; i < cap_capacity(); ++i) {
      cap_snapshot_t snap;
      if (cap_snapshot_at(i, &snap) == 0) {
        continue;
      }

      if (cap_require(snap.handle, snap.type, rights, 0) == CAP_REQUIRE_OK) {
        out->handle = snap.handle;
        return 1;
      }
    }

    out->status = CLI_VALIDATE_DENY;
    out->reason = cap_deny_reason();
  }

  return 1;
}
