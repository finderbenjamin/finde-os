#include <stdint.h>

#include "vga.h"

#define VGA_WIDTH 80u
#define VGA_HEIGHT 25u
#define VGA_TEXT_BUFFER ((volatile uint16_t*)0xB8000u)
#define VGA_COLOR 0x0Fu

static uint32_t g_row = 0;
static uint32_t g_col = 0;

static uint16_t vga_cell(char c) {
  return (uint16_t)(((uint16_t)VGA_COLOR << 8) | (uint8_t)c);
}

static void vga_scroll(void) {
  uint32_t row;
  uint32_t col;

  for (row = 1; row < VGA_HEIGHT; ++row) {
    for (col = 0; col < VGA_WIDTH; ++col) {
      VGA_TEXT_BUFFER[(row - 1u) * VGA_WIDTH + col] = VGA_TEXT_BUFFER[row * VGA_WIDTH + col];
    }
  }

  for (col = 0; col < VGA_WIDTH; ++col) {
    VGA_TEXT_BUFFER[(VGA_HEIGHT - 1u) * VGA_WIDTH + col] = vga_cell(' ');
  }

  g_row = VGA_HEIGHT - 1u;
  g_col = 0;
}

void vga_clear(void) {
  uint32_t i;

  for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
    VGA_TEXT_BUFFER[i] = vga_cell(' ');
  }

  g_row = 0;
  g_col = 0;
}

void vga_init(void) {
  vga_clear();
}

void vga_putc(char c) {
  if (c == '\b') {
    if (g_col == 0 && g_row == 0) {
      return;
    }

    if (g_col > 0) {
      --g_col;
    } else {
      --g_row;
      g_col = VGA_WIDTH - 1u;
    }

    VGA_TEXT_BUFFER[g_row * VGA_WIDTH + g_col] = vga_cell(' ');
    return;
  }

  if (c == '\n') {
    g_col = 0;
    ++g_row;
    if (g_row >= VGA_HEIGHT) {
      vga_scroll();
    }
    return;
  }

  VGA_TEXT_BUFFER[g_row * VGA_WIDTH + g_col] = vga_cell(c);
  ++g_col;

  if (g_col >= VGA_WIDTH) {
    g_col = 0;
    ++g_row;
  }

  if (g_row >= VGA_HEIGHT) {
    vga_scroll();
  }
}

void vga_write(const char* s) {
  while (*s) {
    vga_putc(*s++);
  }
}
