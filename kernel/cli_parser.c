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
  ast_out->arg1 = 0;
  ast_out->arg2 = 0;
  ast_out->arg3 = 0;
  ast_out->arg4 = 0;
  ast_out->arg5 = 0;
  ast_out->arg6 = 0;

  if (buffer->token_count == 0) {
    ast_out->kind = CLI_AST_EMPTY;
    return 1;
  }

  if (streq(buffer->tokens[0], "help")) {
    ast_out->kind = CLI_AST_HELP;
    if (buffer->token_count >= 2) {
      ast_out->arg0 = buffer->tokens[1];
    }
    return 1;
  }

  if (buffer->token_count >= 2 && streq(buffer->tokens[1], "--help")) {
    if (streq(buffer->tokens[0], "status") || streq(buffer->tokens[0], "ticks") || streq(buffer->tokens[0], "malloc") ||
        streq(buffer->tokens[0], "cap") || streq(buffer->tokens[0], "job") || streq(buffer->tokens[0], "hub") || streq(buffer->tokens[0], "home") ||
        streq(buffer->tokens[0], "welcome") || streq(buffer->tokens[0], "onboarding") || streq(buffer->tokens[0], "find") ||
        streq(buffer->tokens[0], "search") || streq(buffer->tokens[0], "session") || streq(buffer->tokens[0], "help")) {
      ast_out->kind = CLI_AST_HELP;
      ast_out->arg0 = buffer->tokens[0];
      return 1;
    }
  }

  if (streq(buffer->tokens[0], "welcome") || streq(buffer->tokens[0], "onboarding")) {
    ast_out->kind = CLI_AST_ONBOARDING;
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

    if (buffer->token_count >= 3 && streq(buffer->tokens[1], "explain")) {
      ast_out->kind = CLI_AST_CAP_EXPLAIN;
      ast_out->arg0 = buffer->tokens[2];
      return 1;
    }
  }

  if (streq(buffer->tokens[0], "job")) {
    ast_out->kind = CLI_AST_JOB;
    if (buffer->token_count >= 2) ast_out->arg0 = buffer->tokens[1];
    if (buffer->token_count >= 3) ast_out->arg1 = buffer->tokens[2];
    if (buffer->token_count >= 4) ast_out->arg2 = buffer->tokens[3];
    if (buffer->token_count >= 5) ast_out->arg3 = buffer->tokens[4];
    if (buffer->token_count >= 6) ast_out->arg4 = buffer->tokens[5];
    if (buffer->token_count >= 7) ast_out->arg5 = buffer->tokens[6];
    if (buffer->token_count >= 8) ast_out->arg6 = buffer->tokens[7];
    return 1;
  }

  if (streq(buffer->tokens[0], "find")) {
    ast_out->kind = CLI_AST_FIND;
    if (buffer->token_count >= 2) ast_out->arg0 = buffer->tokens[1];
    if (buffer->token_count >= 3) ast_out->arg1 = buffer->tokens[2];
    if (buffer->token_count >= 4) ast_out->arg2 = buffer->tokens[3];
    if (buffer->token_count >= 5) ast_out->arg3 = buffer->tokens[4];
    if (buffer->token_count >= 6) ast_out->arg4 = buffer->tokens[5];
    if (buffer->token_count >= 7) ast_out->arg5 = buffer->tokens[6];
    if (buffer->token_count >= 8) ast_out->arg6 = buffer->tokens[7];
    return 1;
  }

  if (streq(buffer->tokens[0], "search")) {
    ast_out->kind = CLI_AST_SEARCH;
    if (buffer->token_count >= 2) ast_out->arg0 = buffer->tokens[1];
    if (buffer->token_count >= 3) ast_out->arg1 = buffer->tokens[2];
    if (buffer->token_count >= 4) ast_out->arg2 = buffer->tokens[3];
    if (buffer->token_count >= 5) ast_out->arg3 = buffer->tokens[4];
    if (buffer->token_count >= 6) ast_out->arg4 = buffer->tokens[5];
    if (buffer->token_count >= 7) ast_out->arg5 = buffer->tokens[6];
    if (buffer->token_count >= 8) ast_out->arg6 = buffer->tokens[7];
    return 1;
  }

  if (streq(buffer->tokens[0], "session")) {
    ast_out->kind = CLI_AST_SESSION;
    if (buffer->token_count >= 2) ast_out->arg0 = buffer->tokens[1];
    if (buffer->token_count >= 3) ast_out->arg1 = buffer->tokens[2];
    if (buffer->token_count >= 4) ast_out->arg2 = buffer->tokens[3];
    if (buffer->token_count >= 5) ast_out->arg3 = buffer->tokens[4];
    if (buffer->token_count >= 6) ast_out->arg4 = buffer->tokens[5];
    if (buffer->token_count >= 7) ast_out->arg5 = buffer->tokens[6];
    if (buffer->token_count >= 8) ast_out->arg6 = buffer->tokens[7];
    return 1;
  }

  if (streq(buffer->tokens[0], "hub") || streq(buffer->tokens[0], "home")) {
    ast_out->kind = CLI_AST_HUB;
    if (buffer->token_count >= 2) ast_out->arg0 = buffer->tokens[1];
    if (buffer->token_count >= 3) ast_out->arg1 = buffer->tokens[2];
    return 1;
  }

  if (streq(buffer->tokens[0], "j")) {
    ast_out->kind = CLI_AST_HUB;
    ast_out->arg0 = "jobs";
    return 1;
  }

  if (streq(buffer->tokens[0], "l")) {
    ast_out->kind = CLI_AST_HUB;
    ast_out->arg0 = "logs";
    return 1;
  }

  if (streq(buffer->tokens[0], "r")) {
    ast_out->kind = CLI_AST_HUB;
    ast_out->arg0 = "retry";
    return 1;
  }

  if (streq(buffer->tokens[0], "q")) {
    ast_out->kind = CLI_AST_HUB;
    ast_out->arg0 = "quit";
    return 1;
  }

  ast_out->arg0 = buffer->tokens[0];
  return 1;
}
