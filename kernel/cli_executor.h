#ifndef KERNEL_CLI_EXECUTOR_H
#define KERNEL_CLI_EXECUTOR_H

#include <stdint.h>

#include "cli_validator.h"

void cli_execute_validated(const cli_validated_command_t* cmd);

#endif
