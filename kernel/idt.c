#include <stdint.h>

#include "idt.h"
#include "serial.h"

struct __attribute__((packed)) idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t zero;
};

struct __attribute__((packed)) idtr {
  uint16_t limit;
  uint64_t base;
};

static struct idt_entry idt[256];

static uint16_t read_cs(void) {
  uint16_t cs;
  __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
  return cs;
}


extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_8(void);
extern void isr_9(void);
extern void isr_10(void);
extern void isr_11(void);
extern void isr_12(void);
extern void isr_13(void);
extern void isr_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_17(void);
extern void isr_18(void);
extern void isr_19(void);
extern void isr_20(void);
extern void isr_21(void);
extern void isr_22(void);
extern void isr_23(void);
extern void isr_24(void);
extern void isr_25(void);
extern void isr_26(void);
extern void isr_27(void);
extern void isr_28(void);
extern void isr_29(void);
extern void isr_30(void);
extern void isr_31(void);

static void set_gate(int vec, void (*handler)(void), uint8_t flags, uint16_t selector) {
  uint64_t addr = (uint64_t)handler;

  idt[vec].offset_low = (uint16_t)(addr & 0xFFFF);
  idt[vec].selector = selector;
  idt[vec].ist = 0;
  idt[vec].type_attr = flags;
  idt[vec].offset_mid = (uint16_t)((addr >> 16) & 0xFFFF);
  idt[vec].offset_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
  idt[vec].zero = 0;
}

void idt_init(void) {
  const uint8_t interrupt_gate = 0xEE;
  const uint16_t selector = read_cs();
  struct idtr desc;
  void (*handlers[32])(void) = {
    isr_0, isr_1, isr_2, isr_3, isr_4, isr_5, isr_6, isr_7,
    isr_8, isr_9, isr_10, isr_11, isr_12, isr_13, isr_14, isr_15,
    isr_16, isr_17, isr_18, isr_19, isr_20, isr_21, isr_22, isr_23,
    isr_24, isr_25, isr_26, isr_27, isr_28, isr_29, isr_30, isr_31,
  };

  for (int i = 0; i < 32; ++i) {
    set_gate(i, handlers[i], interrupt_gate, selector);
  }

  desc.limit = (uint16_t)(sizeof(idt) - 1);
  desc.base = (uint64_t)&idt[0];

  __asm__ volatile ("lidt %0" : : "m"(desc));

  serial_write("IDT_OK\n");
}
