#ifndef KERNEL_VGA_H
#define KERNEL_VGA_H

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_write(const char* s);

#endif
