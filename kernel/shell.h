#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

#include <stddef.h>

void shell_init(void);
void shell_init_minimal(void);
void shell_step(void);

void shell_process_input_char_for_test(char c);
const char* shell_current_line_for_test(void);
size_t shell_cursor_for_test(void);

#endif
