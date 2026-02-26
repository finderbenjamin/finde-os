#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_init(void);
void interrupts_init(void);
uint64_t timer_ticks_get(void);

#endif
