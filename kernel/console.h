#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

void console_init(void);
void console_write(const char* s);
void console_write_char(char c);
int console_has_char(void);
char console_read_char(void);

#endif
