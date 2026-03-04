#include "cli_parser.h"

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

static int cli_tokenize(const char* line, cli_parse_buffer_t* buffer) {
  size_t i = 0;
  size_t count = 0;

  while (line[i] != '\0' && i < CLI_MAX_LINE) {
    buffer->scratch[i] = line[i];
    ++i;
  }

  if (line[i] != '\0') {
    return 0;
  }

  buffer->scratch[i] = '\0';
  i = 0;

  while (buffer->scratch[i] != '\0' && count < CLI_MAX_TOKENS) {
    while (buffer->scratch[i] == ' ') {
      ++i;
    }

    if (buffer->scratch[i] == '\0') {
      break;
    }

    buffer->tokens[count++] = &buffer->scratch[i];
    while (buffer->scratch[i] != '\0' && buffer->scratch[i] != ' ') {
      ++i;
    }

    if (buffer->scratch[i] == '\0') {
      break;
    }

    buffer->scratch[i] = '\0';
    ++i;
  }

  buffer->token_count = count;
  return 1;
}

int cli_parse_line(const char* line, cli_parse_buffer_t* buffer, cli_ast_t* ast_out) {
  if (line == 0 || buffer == 0 || ast_out == 0) {
    return 0;
  }

  if (!cli_tokenize(line, buffer)) {
    return 0;
  }

  ast_out->kind = CLI_AST_UNKNOWN;
  ast_out->arg0 = 0;

  if (buffer->token_count == 0) {
    ast_out->kind = CLI_AST_EMPTY;
    return 1;
  }

  if (streq(buffer->tokens[0], "help")) {
    ast_out->kind = CLI_AST_HELP;
    return 1;
  }

  if (streq(buffer->tokens[0], "status")) {
    ast_out->kind = CLI_AST_STATUS;
    return 1;
  }

  if (streq(buffer->tokens[0], "ticks")) {
    ast_out->kind = CLI_AST_TICKS;
    return 1;
  }

  if (streq(buffer->tokens[0], "malloc")) {
    ast_out->kind = CLI_AST_MALLOC;
    return 1;
  }

  if (streq(buffer->tokens[0], "cap")) {
    if (buffer->token_count >= 2 && streq(buffer->tokens[1], "list")) {
      ast_out->kind = CLI_AST_CAP_LIST;
      return 1;
    }

    if (buffer->token_count >= 3 && streq(buffer->tokens[1], "show")) {
      ast_out->kind = CLI_AST_CAP_SHOW;
      ast_out->arg0 = buffer->tokens[2];
      return 1;
    }

    if (buffer->token_count >= 3 && streq(buffer->tokens[1], "check")) {
      ast_out->kind = CLI_AST_CAP_CHECK;
      ast_out->arg0 = buffer->tokens[2];
      return 1;
    }
  }

  return 1;
}
