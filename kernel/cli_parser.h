#ifndef KERNEL_CLI_PARSER_H
#define KERNEL_CLI_PARSER_H

#include <stddef.h>

#define CLI_MAX_LINE 128
#define CLI_MAX_TOKENS 8

typedef enum {
  CLI_AST_EMPTY = 0,
  CLI_AST_HELP,
  CLI_AST_STATUS,
  CLI_AST_TICKS,
  CLI_AST_MALLOC,
  CLI_AST_CAP_LIST,
  CLI_AST_CAP_SHOW,
  CLI_AST_CAP_CHECK,
  CLI_AST_UNKNOWN,
} cli_ast_kind_t;

typedef struct {
  cli_ast_kind_t kind;
  const char* arg0;
} cli_ast_t;

typedef struct {
  char scratch[CLI_MAX_LINE + 1];
  char* tokens[CLI_MAX_TOKENS];
  size_t token_count;
} cli_parse_buffer_t;

int cli_parse_line(const char* line, cli_parse_buffer_t* buffer, cli_ast_t* ast_out);

#endif
