#include "shell.h"

#include "cli_executor.h"
#include "cli_parser.h"
#include "cli_validator.h"
#include "console.h"

static char g_line[CLI_MAX_LINE + 1];
static size_t g_cursor = 0;
static cli_mode_t g_mode = CLI_MODE_SANDBOX;

static void shell_prompt(void) {
  console_write("finde-os> ");
}

void shell_set_mode_for_test(cli_mode_t mode) {
  g_mode = mode;
}

void shell_execute_line_for_test(const char* line) {
  cli_parse_buffer_t parse_buffer;
  cli_ast_t ast;
  cli_validated_command_t validated;

  if (!cli_parse_line(line, &parse_buffer, &ast)) {
    console_write("CLI_ERROR stage=parse reason=line-too-long\n");
    return;
  }

  if (!cli_validate_ast(&ast, g_mode, &validated)) {
    console_write("CLI_ERROR stage=validate reason=internal\n");
    return;
  }

  if (validated.status == CLI_VALIDATE_SYNTAX) {
    console_write("CLI_ERROR stage=validate reason=");
    console_write(validated.reason);
    console_write("\n");
    return;
  }

  if (validated.status == CLI_VALIDATE_DENY) {
    if (ast.kind == CLI_AST_CAP_SHOW) {
      console_write("CAP_SHOW deny=");
      console_write(validated.reason);
      console_write("\n");
      return;
    }

    if (ast.kind == CLI_AST_CAP_CHECK) {
      console_write("CAP_CHECK op=");
      if (ast.arg0 != 0) {
        console_write(ast.arg0);
      }
      console_write(" result=DENY reason=");
      console_write(validated.reason);
      console_write("\n");
      return;
    }

    console_write("CLI_DENY reason=");
    console_write(validated.reason);
    console_write("\n");
    return;
  }

  cli_execute_validated(&validated);
}

void shell_init_minimal(void) {
  g_cursor = 0;
  g_mode = CLI_MODE_SANDBOX;
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
    if (g_cursor < CLI_MAX_LINE) {
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
