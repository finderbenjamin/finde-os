#ifndef KERNEL_KEYBOARD_H
#define KERNEL_KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
void keyboard_handle_scancode(uint8_t scancode);
int keyboard_has_char(void);
char keyboard_read_char(void);
char keyboard_decode_scancode_for_test(uint8_t scancode, int shift);

#endif
