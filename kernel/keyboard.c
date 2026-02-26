#include "keyboard.h"

#include <stdint.h>

#define KBD_QUEUE_SIZE 256u
#define KBD_SCANCODE_RELEASE_MASK 0x80u
#define KBD_SC_LSHIFT 0x2Au
#define KBD_SC_RSHIFT 0x36u

static char g_queue[KBD_QUEUE_SIZE];
static uint32_t g_head = 0;
static uint32_t g_tail = 0;
static int g_shift = 0;

char keyboard_decode_scancode_for_test(uint8_t scancode, int shift) {
  switch (scancode) {
    case 0x02: return shift ? '!' : '1';
    case 0x03: return shift ? '@' : '2';
    case 0x04: return shift ? '#' : '3';
    case 0x05: return shift ? '$' : '4';
    case 0x06: return shift ? '%' : '5';
    case 0x07: return shift ? '^' : '6';
    case 0x08: return shift ? '&' : '7';
    case 0x09: return shift ? '*' : '8';
    case 0x0A: return shift ? '(' : '9';
    case 0x0B: return shift ? ')' : '0';
    case 0x0C: return shift ? '_' : '-';
    case 0x0D: return shift ? '+' : '=';
    case 0x0E: return '\b';
    case 0x10: return shift ? 'Q' : 'q';
    case 0x11: return shift ? 'W' : 'w';
    case 0x12: return shift ? 'E' : 'e';
    case 0x13: return shift ? 'R' : 'r';
    case 0x14: return shift ? 'T' : 't';
    case 0x15: return shift ? 'Y' : 'y';
    case 0x16: return shift ? 'U' : 'u';
    case 0x17: return shift ? 'I' : 'i';
    case 0x18: return shift ? 'O' : 'o';
    case 0x19: return shift ? 'P' : 'p';
    case 0x1A: return shift ? '{' : '[';
    case 0x1B: return shift ? '}' : ']';
    case 0x1C: return '\n';
    case 0x1E: return shift ? 'A' : 'a';
    case 0x1F: return shift ? 'S' : 's';
    case 0x20: return shift ? 'D' : 'd';
    case 0x21: return shift ? 'F' : 'f';
    case 0x22: return shift ? 'G' : 'g';
    case 0x23: return shift ? 'H' : 'h';
    case 0x24: return shift ? 'J' : 'j';
    case 0x25: return shift ? 'K' : 'k';
    case 0x26: return shift ? 'L' : 'l';
    case 0x27: return shift ? ':' : ';';
    case 0x28: return shift ? '"' : '\'';
    case 0x2B: return shift ? '|' : '\\';
    case 0x2C: return shift ? 'Z' : 'z';
    case 0x2D: return shift ? 'X' : 'x';
    case 0x2E: return shift ? 'C' : 'c';
    case 0x2F: return shift ? 'V' : 'v';
    case 0x30: return shift ? 'B' : 'b';
    case 0x31: return shift ? 'N' : 'n';
    case 0x32: return shift ? 'M' : 'm';
    case 0x33: return shift ? '<' : ',';
    case 0x34: return shift ? '>' : '.';
    case 0x35: return shift ? '?' : '/';
    case 0x39: return ' ';
    default: return 0;
  }
}

static void queue_push(char c) {
  uint32_t next = (g_head + 1u) & (KBD_QUEUE_SIZE - 1u);
  if (next == g_tail) {
    return;
  }

  g_queue[g_head] = c;
  g_head = next;
}

void keyboard_init(void) {
  g_head = 0;
  g_tail = 0;
  g_shift = 0;
}

void keyboard_handle_scancode(uint8_t scancode) {
  if (scancode == KBD_SC_LSHIFT || scancode == KBD_SC_RSHIFT) {
    g_shift = 1;
    return;
  }

  if (scancode == (KBD_SC_LSHIFT | KBD_SCANCODE_RELEASE_MASK) ||
      scancode == (KBD_SC_RSHIFT | KBD_SCANCODE_RELEASE_MASK)) {
    g_shift = 0;
    return;
  }

  if ((scancode & KBD_SCANCODE_RELEASE_MASK) != 0) {
    return;
  }

  char c = keyboard_decode_scancode_for_test(scancode, g_shift);
  if (c != 0) {
    queue_push(c);
  }
}

int keyboard_has_char(void) {
  return g_head != g_tail;
}

char keyboard_read_char(void) {
  while (!keyboard_has_char()) {
  }

  char c = g_queue[g_tail];
  g_tail = (g_tail + 1u) & (KBD_QUEUE_SIZE - 1u);
  return c;
}
