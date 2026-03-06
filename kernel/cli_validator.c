#include "cli_validator.h"

#include "cap.h"
#include "job.h"

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

static const char* suggest_command(const char* command) {
  if (command == 0 || *command == '\0') {
    return "help";
  }

  if (command[0] == 'h') return "help";
  if (command[0] == 'w' || command[0] == 'o') return "welcome";
  if (command[0] == 's') return "status";
  if (command[0] == 't') return "ticks";
  if (command[0] == 'm') return "malloc";
  if (command[0] == 'c') return "cap";
  if (command[0] == 'j') return "job";
  if (command[0] == 'f') return "find";
  return "help";
}

static const char* ast_arg_at(const cli_ast_t* ast, int idx) {
  if (idx == 0) return ast->arg0;
  if (idx == 1) return ast->arg1;
  if (idx == 2) return ast->arg2;
  if (idx == 3) return ast->arg3;
  if (idx == 4) return ast->arg4;
  if (idx == 5) return ast->arg5;
  if (idx == 6) return ast->arg6;
  return 0;
}

static int parse_common_options(const cli_ast_t* ast, int start_idx, cli_validated_command_t* out, const char** reason_out) {
  int i = start_idx;
  while (i <= 6) {
    const char* arg = ast_arg_at(ast, i);
    if (arg == 0) {
      return 1;
    }

    if (streq(arg, "--json")) {
      out->json_output = 1;
      i += 1;
      continue;
    }

    if (streq(arg, "--limit")) {
      uint64_t parsed = 0;
      const char* val = ast_arg_at(ast, i + 1);
      if (!parse_u64(val, &parsed) || parsed == 0u) {
        *reason_out = "--limit requires positive number";
        return 0;
      }
      out->limit = parsed;
      i += 2;
      continue;
    }

    if (streq(arg, "--path")) {
      const char* val = ast_arg_at(ast, i + 1);
      if (val == 0 || *val == '\0') {
        *reason_out = "--path requires a value";
        return 0;
      }
      out->path = val;
      i += 2;
      continue;
    }

    *reason_out = "unknown option";
    return 0;
  }

  return 1;
}

int cli_validate_ast(const cli_ast_t* ast, cli_mode_t mode, cli_validated_command_t* out) {
  if (ast == 0 || out == 0) {
    return 0;
  }

  out->ast = *ast;
  out->status = CLI_VALIDATE_OK;
  out->reason = "OK";
  out->handle = 0;
  out->job_id = 0;
  out->follow = 0;
  out->profile = JOB_PROFILE_ISOLATED;
  out->mode = mode;
  out->suggestion = 0;
  out->json_output = 0;
  out->limit = 10u;
  out->path = ".";
  out->target = 0;

  if (ast->kind == CLI_AST_EMPTY) {
    return 1;
  }

  if (ast->kind == CLI_AST_UNKNOWN) {
    out->status = CLI_VALIDATE_SYNTAX;
    out->reason = "unknown command";
    out->suggestion = suggest_command(ast->arg0);
    return 1;
  }

  if (ast->kind == CLI_AST_HELP && ast->arg0 != 0) {
    if (streq(ast->arg0, "help") || streq(ast->arg0, "status") || streq(ast->arg0, "ticks") || streq(ast->arg0, "malloc") ||
        streq(ast->arg0, "cap") || streq(ast->arg0, "job") || streq(ast->arg0, "hub") || streq(ast->arg0, "home") ||
        streq(ast->arg0, "welcome") || streq(ast->arg0, "onboarding") || streq(ast->arg0, "find") || streq(ast->arg0, "search") ||
        streq(ast->arg0, "session")) {
      return 1;
    }

    out->status = CLI_VALIDATE_SYNTAX;
    out->reason = "unknown help topic";
    out->suggestion = suggest_command(ast->arg0);
    return 1;
  }

  if (ast->kind == CLI_AST_FIND || ast->kind == CLI_AST_SEARCH) {
    const char* reason = 0;
    if (ast->arg0 == 0 || *ast->arg0 == '\0') {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = ast->kind == CLI_AST_FIND ? "find requires <pattern>" : "search requires <text>";
      return 1;
    }

    out->target = ast->arg0;
    if (!parse_common_options(ast, 1, out, &reason)) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = reason;
      return 1;
    }
    return 1;
  }

  if (ast->kind == CLI_AST_SESSION) {
    const char* reason = 0;
    if (ast->arg0 == 0 || (!streq(ast->arg0, "save") && !streq(ast->arg0, "restore"))) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "session requires save|restore";
      return 1;
    }
    if (ast->arg1 == 0 || *ast->arg1 == '\0') {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "session save|restore requires <name>";
      return 1;
    }

    out->target = ast->arg1;
    if (!parse_common_options(ast, 2, out, &reason)) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = reason;
      return 1;
    }
    return 1;
  }

  if (ast->kind == CLI_AST_HUB) {
    if (ast->arg0 == 0) {
      return 1;
    }

    if (streq(ast->arg0, "jobs") || streq(ast->arg0, "commands") || streq(ast->arg0, "errors") || streq(ast->arg0, "logs") ||
        streq(ast->arg0, "status") || streq(ast->arg0, "retry") || streq(ast->arg0, "quit")) {
      return 1;
    }

    out->status = CLI_VALIDATE_SYNTAX;
    out->reason = "hub action unknown";
    return 1;
  }

  if (ast->kind == CLI_AST_JOB) {
    if (ast->arg0 == 0) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "job requires subcommand";
      return 1;
    }

    if (streq(ast->arg0, "list")) {
      return 1;
    }

    if (streq(ast->arg0, "start")) {
      if (ast->arg1 == 0 || *ast->arg1 == '\0') {
        out->status = CLI_VALIDATE_SYNTAX;
        out->reason = "job start requires <cmd>";
        return 1;
      }

      if (ast->arg2 == 0) {
        out->profile = JOB_PROFILE_ISOLATED;
        return 1;
      }

      if (ast->arg2[0] == '-' && ast->arg2[1] == '-' && ast->arg2[2] == 'p' && ast->arg2[3] == 'r' && ast->arg2[4] == 'o' &&
          ast->arg2[5] == 'f' && ast->arg2[6] == 'i' && ast->arg2[7] == 'l' && ast->arg2[8] == 'e' && ast->arg2[9] == '=') {
        if (job_profile_from_flag(&ast->arg2[10], &out->profile)) {
          return 1;
        }

        out->status = CLI_VALIDATE_SYNTAX;
        out->reason = "job start profile must be --profile=default|isolated";
        return 1;
      }

      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "job start supports only optional --profile=default|isolated";
      return 1;
    }

    if (streq(ast->arg0, "status") || streq(ast->arg0, "logs") || streq(ast->arg0, "stop")) {
      if (!parse_u64(ast->arg1, &out->job_id)) {
        out->status = CLI_VALIDATE_SYNTAX;
        out->reason = "job id must be numeric";
        return 1;
      }

      if (streq(ast->arg0, "logs") && ast->arg2 != 0) {
        if (streq(ast->arg2, "--follow")) {
          out->follow = 1;
          return 1;
        }

        out->status = CLI_VALIDATE_SYNTAX;
        out->reason = "job logs supports only --follow";
      }
      return 1;
    }

    out->status = CLI_VALIDATE_SYNTAX;
    out->reason = "unknown job subcommand";
    return 1;
  }

  if (ast->kind == CLI_AST_MALLOC && mode == CLI_MODE_MICROVM) {
    out->status = CLI_VALIDATE_DENY;
    out->reason = "DENY: command not allowed in microvm mode";
    return 1;
  }

  if (ast->kind == CLI_AST_CAP_EXPLAIN) {
    if (!job_profile_from_flag(ast->arg0, &out->profile)) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "cap explain requires profile default|isolated";
      return 1;
    }
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
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "cap check requires read|write|exec";
      return 1;
    }

    out->handle = handle;
    return 1;
  }

  if (ast->kind == CLI_AST_CAP_CHECK) {
    uint32_t rights = op_to_rights(ast->arg0);
    if (rights == 0u) {
      out->status = CLI_VALIDATE_SYNTAX;
      out->reason = "cap check requires read|write|exec";
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
