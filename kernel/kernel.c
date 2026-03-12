#include <stddef.h>
#include <stdint.h>

#define COM1 0x3F8
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F

static volatile uint16_t* const VGA_MEMORY = (volatile uint16_t*)0xB8000;
static size_t terminal_row = 0;
static const char DEMO_LINE_DIGITS[] =
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829";

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static uint16_t vga_entry(char c) {
  return (uint16_t)(uint8_t)c | ((uint16_t)VGA_COLOR << 8);
}

static void serial_init(void) {
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 0x03);
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x03);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);
}

static int serial_tx_empty(void) {
  return inb(COM1 + 5) & 0x20;
}

static void serial_write_char(char c) {
  while (!serial_tx_empty()) { }
  outb(COM1, (uint8_t)c);
}

static void serial_write(const char* s) {
  while (*s) {
    serial_write_char(*s++);
  }
}

static void terminal_clear_row(size_t row) {
  size_t column;

  for (column = 0; column < VGA_WIDTH; ++column) {
    VGA_MEMORY[row * VGA_WIDTH + column] = vga_entry(' ');
  }
}

static void terminal_initialize(void) {
  size_t row;

  terminal_row = 0;

  for (row = 0; row < VGA_HEIGHT; ++row) {
    terminal_clear_row(row);
  }
}

__attribute__((noinline)) static void terminal_scroll(void) {
  size_t row;
  size_t column;

  for (row = 1; row < VGA_HEIGHT; ++row) {
    for (column = 0; column < VGA_WIDTH; ++column) {
      VGA_MEMORY[(row - 1) * VGA_WIDTH + column] =
          VGA_MEMORY[row * VGA_WIDTH + column];
    }
  }

  terminal_clear_row(VGA_HEIGHT - 1);
}

__attribute__((noinline)) static void terminal_write_text_at(size_t row, const char* text) {
  size_t column = 0;

  while (text[column] != '\0' && column < VGA_WIDTH) {
    VGA_MEMORY[row * VGA_WIDTH + column] = vga_entry(text[column]);
    ++column;
  }
}

__attribute__((noinline)) static void terminal_write_u8_2_at(size_t row, size_t column, size_t value) {
  size_t offset;

  if (column + 1 >= VGA_WIDTH) {
    return;
  }

  if (value >= 30) {
    value = 29;
  }

  offset = value * 2;
  VGA_MEMORY[row * VGA_WIDTH + column] = vga_entry(DEMO_LINE_DIGITS[offset]);
  VGA_MEMORY[row * VGA_WIDTH + column + 1] = vga_entry(DEMO_LINE_DIGITS[offset + 1]);
}

__attribute__((noinline)) static void terminal_write_line_with_number(const char* prefix, size_t value) {
  if (terminal_row >= VGA_HEIGHT) {
    terminal_scroll();
    terminal_row = VGA_HEIGHT - 1;
  }

  terminal_clear_row(terminal_row);
  terminal_write_text_at(terminal_row, prefix);
  terminal_write_u8_2_at(terminal_row, 14, value);
  ++terminal_row;
}

static void terminal_demo_scroll(void) {
  size_t line;

  for (line = 0; line < 30; ++line) {
    terminal_write_line_with_number("finde-os line ", line);
  }
}

void kernel_main(void) {
  serial_init();
  terminal_initialize();
  terminal_demo_scroll();

  serial_write("BOOT_OK\n");
  serial_write("SCROLL_OK\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
