#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

void serial_init(void);
int serial_received(void);
char serial_read_char(void);
void serial_write_char(char c);
void serial_write(const char* s);

#endif
