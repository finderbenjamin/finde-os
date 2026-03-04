#ifndef KERNEL_CLI_VALIDATOR_H
#define KERNEL_CLI_VALIDATOR_H

#include <stdint.h>

#include "cli_parser.h"

typedef enum {
  CLI_MODE_SANDBOX = 0,
  CLI_MODE_MICROVM = 1,
} cli_mode_t;

typedef enum {
  CLI_VALIDATE_OK = 0,
  CLI_VALIDATE_SYNTAX,
  CLI_VALIDATE_DENY,
} cli_validate_status_t;

typedef struct {
  cli_ast_t ast;
  cli_validate_status_t status;
  const char* reason;
  uint64_t handle;
  cli_mode_t mode;
} cli_validated_command_t;

int cli_validate_ast(const cli_ast_t* ast, cli_mode_t mode, cli_validated_command_t* out);

#endif
