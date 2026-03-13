#include <stddef.h>
#include <stdint.h>

#define COM1 0x3F8u

#define CONSOLE_COLS 80u
#define CONSOLE_ROWS 25u
#define GLYPH_WIDTH 8u
#define GLYPH_HEIGHT 8u
#define CHAR_HEIGHT 16u
#define TEXT_AREA_WIDTH (CONSOLE_COLS * GLYPH_WIDTH)
#define TEXT_AREA_HEIGHT (CONSOLE_ROWS * CHAR_HEIGHT)

#define DEMO_LINE_COUNT 30u

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u
#define MULTIBOOT2_TAG_TYPE_END 0u
#define MULTIBOOT2_TAG_TYPE_FRAMEBUFFER 8u
#define MULTIBOOT2_FRAMEBUFFER_TYPE_RGB 1u

typedef struct __attribute__((packed)) {
  uint32_t total_size;
  uint32_t reserved;
} multiboot2_info_t;

typedef struct __attribute__((packed)) {
  uint32_t type;
  uint32_t size;
} multiboot2_tag_t;

typedef struct __attribute__((packed)) {
  uint32_t type;
  uint32_t size;
  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
  uint8_t framebuffer_type;
  uint16_t reserved;
  uint8_t red_field_position;
  uint8_t red_mask_size;
  uint8_t green_field_position;
  uint8_t green_mask_size;
  uint8_t blue_field_position;
  uint8_t blue_mask_size;
} multiboot2_tag_framebuffer_t;

static volatile uint8_t* g_framebuffer = (volatile uint8_t*)0;
static uint32_t g_framebuffer_pitch = 0;
static uint32_t g_framebuffer_width = 0;
static uint32_t g_framebuffer_height = 0;
static uint8_t g_framebuffer_bpp = 0;
static uint8_t g_framebuffer_bytes_per_pixel = 0;
static uint8_t g_red_field_position = 0;
static uint8_t g_red_mask_size = 0;
static uint8_t g_green_field_position = 0;
static uint8_t g_green_mask_size = 0;
static uint8_t g_blue_field_position = 0;
static uint8_t g_blue_mask_size = 0;
static uint32_t g_text_row = 0;
static char g_shadow[CONSOLE_ROWS][CONSOLE_COLS];

static uint32_t g_color_black = 0;
static uint32_t g_color_white = 0;

static const char DEMO_PREFIX[] = "finde-os line ";
static const char DEMO_LINE_DIGITS[] =
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829";

static const uint8_t GLYPH_SPACE[GLYPH_HEIGHT] = {0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t GLYPH_HYPHEN[GLYPH_HEIGHT] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00};
static const uint8_t GLYPH_QUESTION[GLYPH_HEIGHT] = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00};

static const uint8_t GLYPH_0[GLYPH_HEIGHT] = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_1[GLYPH_HEIGHT] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00};
static const uint8_t GLYPH_2[GLYPH_HEIGHT] = {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x60, 0x7E, 0x00};
static const uint8_t GLYPH_3[GLYPH_HEIGHT] = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_4[GLYPH_HEIGHT] = {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00};
static const uint8_t GLYPH_5[GLYPH_HEIGHT] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_6[GLYPH_HEIGHT] = {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_7[GLYPH_HEIGHT] = {0x7E, 0x66, 0x0C, 0x18, 0x18, 0x18, 0x18, 0x00};
static const uint8_t GLYPH_8[GLYPH_HEIGHT] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_9[GLYPH_HEIGHT] = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00};

static const uint8_t GLYPH_D[GLYPH_HEIGHT] = {0x0C, 0x0C, 0x3C, 0x6C, 0x6C, 0x6C, 0x36, 0x00};
static const uint8_t GLYPH_E[GLYPH_HEIGHT] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00};
static const uint8_t GLYPH_F[GLYPH_HEIGHT] = {0x1C, 0x30, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x00};
static const uint8_t GLYPH_I[GLYPH_HEIGHT] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00};
static const uint8_t GLYPH_L[GLYPH_HEIGHT] = {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00};
static const uint8_t GLYPH_N[GLYPH_HEIGHT] = {0x00, 0x00, 0x6C, 0x76, 0x66, 0x66, 0x66, 0x00};
static const uint8_t GLYPH_O[GLYPH_HEIGHT] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00};
static const uint8_t GLYPH_S[GLYPH_HEIGHT] = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00};

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static void serial_init(void) {
  outb(COM1 + 1u, 0x00u);
  outb(COM1 + 3u, 0x80u);
  outb(COM1 + 0u, 0x03u);
  outb(COM1 + 1u, 0x00u);
  outb(COM1 + 3u, 0x03u);
  outb(COM1 + 2u, 0xC7u);
  outb(COM1 + 4u, 0x0Bu);
}

static int serial_tx_empty(void) {
  return (inb(COM1 + 5u) & 0x20u) != 0u;
}

static void serial_write_char(char c) {
  while (!serial_tx_empty()) {
  }
  outb(COM1, (uint8_t)c);
}

static void serial_write(const char* s) {
  while (*s != '\0') {
    serial_write_char(*s++);
  }
}

static const multiboot2_tag_t* multiboot2_first_tag(const multiboot2_info_t* info) {
  return (const multiboot2_tag_t*)((const uint8_t*)info + 8u);
}

static const multiboot2_tag_t* multiboot2_next_tag(const multiboot2_tag_t* tag) {
  const uintptr_t next = ((uintptr_t)tag + (uintptr_t)tag->size + 7u) & ~(uintptr_t)7u;
  return (const multiboot2_tag_t*)next;
}

static const multiboot2_tag_framebuffer_t* multiboot2_find_framebuffer_tag(uint64_t mb_magic,
                                                                            uint64_t mb_info_addr) {
  const multiboot2_info_t* info;
  const multiboot2_tag_t* tag;
  const uint8_t* end;

  if ((uint32_t)mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC || mb_info_addr == 0u) {
    return (const multiboot2_tag_framebuffer_t*)0;
  }

  info = (const multiboot2_info_t*)(uintptr_t)mb_info_addr;
  end = (const uint8_t*)info + info->total_size;

  for (tag = multiboot2_first_tag(info);
       (const uint8_t*)tag + sizeof(multiboot2_tag_t) <= end;
       tag = multiboot2_next_tag(tag)) {
    if (tag->type == MULTIBOOT2_TAG_TYPE_END) {
      break;
    }

    if (tag->size < sizeof(multiboot2_tag_t)) {
      break;
    }

    if (tag->type == MULTIBOOT2_TAG_TYPE_FRAMEBUFFER &&
        tag->size >= sizeof(multiboot2_tag_framebuffer_t)) {
      return (const multiboot2_tag_framebuffer_t*)tag;
    }
  }

  return (const multiboot2_tag_framebuffer_t*)0;
}

static uint32_t framebuffer_scale_component(uint8_t value, uint8_t bits) {
  uint32_t max_value;

  if (bits == 0u) {
    return 0u;
  }

  max_value = (1u << bits) - 1u;
  return ((uint32_t)value * max_value + 127u) / 255u;
}

static uint32_t framebuffer_make_color(uint8_t red, uint8_t green, uint8_t blue) {
  uint32_t color = 0u;

  color |= framebuffer_scale_component(red, g_red_mask_size) << g_red_field_position;
  color |= framebuffer_scale_component(green, g_green_mask_size) << g_green_field_position;
  color |= framebuffer_scale_component(blue, g_blue_mask_size) << g_blue_field_position;

  return color;
}

static void framebuffer_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
  volatile uint8_t* pixel;

  if (x >= g_framebuffer_width || y >= g_framebuffer_height) {
    return;
  }

  pixel = g_framebuffer + (size_t)y * g_framebuffer_pitch + (size_t)x * g_framebuffer_bytes_per_pixel;

  if (g_framebuffer_bytes_per_pixel == 4u) {
    *(volatile uint32_t*)pixel = color;
  } else if (g_framebuffer_bytes_per_pixel == 3u) {
    pixel[0] = (uint8_t)(color & 0xFFu);
    pixel[1] = (uint8_t)((color >> 8) & 0xFFu);
    pixel[2] = (uint8_t)((color >> 16) & 0xFFu);
  }
}

static void framebuffer_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
  uint32_t dy;
  uint32_t dx;

  for (dy = 0u; dy < height; ++dy) {
    for (dx = 0u; dx < width; ++dx) {
      framebuffer_put_pixel(x + dx, y + dy, color);
    }
  }
}

static const uint8_t* framebuffer_glyph_for_char(char c) {
  switch (c) {
    case ' ':
      return GLYPH_SPACE;
    case '-':
      return GLYPH_HYPHEN;
    case '0':
      return GLYPH_0;
    case '1':
      return GLYPH_1;
    case '2':
      return GLYPH_2;
    case '3':
      return GLYPH_3;
    case '4':
      return GLYPH_4;
    case '5':
      return GLYPH_5;
    case '6':
      return GLYPH_6;
    case '7':
      return GLYPH_7;
    case '8':
      return GLYPH_8;
    case '9':
      return GLYPH_9;
    case 'd':
      return GLYPH_D;
    case 'e':
      return GLYPH_E;
    case 'f':
      return GLYPH_F;
    case 'i':
      return GLYPH_I;
    case 'l':
      return GLYPH_L;
    case 'n':
      return GLYPH_N;
    case 'o':
      return GLYPH_O;
    case 's':
      return GLYPH_S;
    default:
      return GLYPH_QUESTION;
  }
}

__attribute__((noinline, optnone)) static void framebuffer_draw_char_at(uint32_t row,
                                                                        uint32_t column,
                                                                        char c) {
  const uint8_t* glyph;
  uint32_t x0;
  uint32_t y0;
  uint32_t glyph_row;
  uint32_t glyph_col;
  uint8_t bits;

  if (row >= CONSOLE_ROWS || column >= CONSOLE_COLS) {
    return;
  }

  g_shadow[row][column] = c;

  x0 = column * GLYPH_WIDTH;
  y0 = row * CHAR_HEIGHT;
  glyph = framebuffer_glyph_for_char(c);

  framebuffer_fill_rect(x0, y0, GLYPH_WIDTH, CHAR_HEIGHT, g_color_black);

  for (glyph_row = 0u; glyph_row < GLYPH_HEIGHT; ++glyph_row) {
    bits = glyph[glyph_row];
    for (glyph_col = 0u; glyph_col < GLYPH_WIDTH; ++glyph_col) {
      if ((bits & (uint8_t)(0x80u >> glyph_col)) != 0u) {
        framebuffer_put_pixel(x0 + glyph_col, y0 + glyph_row * 2u, g_color_white);
        framebuffer_put_pixel(x0 + glyph_col, y0 + glyph_row * 2u + 1u, g_color_white);
      }
    }
  }
}

__attribute__((noinline, optnone)) static void terminal_clear_row(uint32_t row) {
  uint32_t column;

  if (row >= CONSOLE_ROWS) {
    return;
  }

  for (column = 0u; column < CONSOLE_COLS; ++column) {
    g_shadow[row][column] = ' ';
  }

  framebuffer_fill_rect(0u, row * CHAR_HEIGHT, TEXT_AREA_WIDTH, CHAR_HEIGHT, g_color_black);
}

__attribute__((noinline, optnone)) static void terminal_redraw_all(void) {
  uint32_t row;
  uint32_t column;

  framebuffer_fill_rect(0u, 0u, TEXT_AREA_WIDTH, TEXT_AREA_HEIGHT, g_color_black);

  for (row = 0u; row < CONSOLE_ROWS; ++row) {
    for (column = 0u; column < CONSOLE_COLS; ++column) {
      if (g_shadow[row][column] != ' ') {
        framebuffer_draw_char_at(row, column, g_shadow[row][column]);
      }
    }
  }
}

__attribute__((noinline, optnone)) static void terminal_scroll(void) {
  uint32_t row;
  uint32_t column;

  for (row = 1u; row < CONSOLE_ROWS; ++row) {
    for (column = 0u; column < CONSOLE_COLS; ++column) {
      g_shadow[row - 1u][column] = g_shadow[row][column];
    }
  }

  for (column = 0u; column < CONSOLE_COLS; ++column) {
    g_shadow[CONSOLE_ROWS - 1u][column] = ' ';
  }

  terminal_redraw_all();
}

__attribute__((noinline, optnone)) static int terminal_initialize(uint64_t mb_magic,
                                                                  uint64_t mb_info_addr) {
  const multiboot2_tag_framebuffer_t* framebuffer_tag;
  uint32_t row;
  uint32_t column;

  framebuffer_tag = multiboot2_find_framebuffer_tag(mb_magic, mb_info_addr);
  if (framebuffer_tag == (const multiboot2_tag_framebuffer_t*)0) {
    serial_write("FB_TAG_MISSING\n");
    return 0;
  }

  if (framebuffer_tag->framebuffer_type != MULTIBOOT2_FRAMEBUFFER_TYPE_RGB) {
    serial_write("FB_NOT_RGB\n");
    return 0;
  }

  g_framebuffer = (volatile uint8_t*)(uintptr_t)framebuffer_tag->framebuffer_addr;
  g_framebuffer_pitch = framebuffer_tag->framebuffer_pitch;
  g_framebuffer_width = framebuffer_tag->framebuffer_width;
  g_framebuffer_height = framebuffer_tag->framebuffer_height;
  g_framebuffer_bpp = framebuffer_tag->framebuffer_bpp;
  g_framebuffer_bytes_per_pixel = (uint8_t)((g_framebuffer_bpp + 7u) / 8u);
  g_red_field_position = framebuffer_tag->red_field_position;
  g_red_mask_size = framebuffer_tag->red_mask_size;
  g_green_field_position = framebuffer_tag->green_field_position;
  g_green_mask_size = framebuffer_tag->green_mask_size;
  g_blue_field_position = framebuffer_tag->blue_field_position;
  g_blue_mask_size = framebuffer_tag->blue_mask_size;

  if (g_framebuffer == (volatile uint8_t*)0 ||
      (g_framebuffer_bytes_per_pixel != 3u && g_framebuffer_bytes_per_pixel != 4u) ||
      g_framebuffer_width < TEXT_AREA_WIDTH ||
      g_framebuffer_height < TEXT_AREA_HEIGHT) {
    serial_write("FB_UNSUPPORTED\n");
    return 0;
  }

  g_color_black = framebuffer_make_color(0u, 0u, 0u);
  g_color_white = framebuffer_make_color(255u, 255u, 255u);
  g_text_row = 0u;

  framebuffer_fill_rect(0u, 0u, g_framebuffer_width, g_framebuffer_height, g_color_black);

  for (row = 0u; row < CONSOLE_ROWS; ++row) {
    for (column = 0u; column < CONSOLE_COLS; ++column) {
      g_shadow[row][column] = ' ';
    }
  }

  return 1;
}

__attribute__((noinline, optnone)) static void terminal_write_text_at(uint32_t row,
                                                                      const char* text) {
  uint32_t column = 0u;

  while (text[column] != '\0' && column < CONSOLE_COLS) {
    framebuffer_draw_char_at(row, column, text[column]);
    ++column;
  }
}

__attribute__((noinline, optnone)) static void terminal_write_u8_2_at(uint32_t row,
                                                                      uint32_t column,
                                                                      uint32_t value) {
  uint32_t offset;

  if (row >= CONSOLE_ROWS || column + 1u >= CONSOLE_COLS) {
    return;
  }

  if (value >= DEMO_LINE_COUNT) {
    value = DEMO_LINE_COUNT - 1u;
  }

  offset = value * 2u;
  framebuffer_draw_char_at(row, column, DEMO_LINE_DIGITS[offset]);
  framebuffer_draw_char_at(row, column + 1u, DEMO_LINE_DIGITS[offset + 1u]);
}

__attribute__((noinline, optnone)) static void terminal_write_line_with_number(const char* prefix,
                                                                               uint32_t value) {
  if (g_text_row >= CONSOLE_ROWS) {
    terminal_scroll();
    g_text_row = CONSOLE_ROWS - 1u;
  }

  terminal_clear_row(g_text_row);
  terminal_write_text_at(g_text_row, prefix);
  terminal_write_u8_2_at(g_text_row, 14u, value);
  ++g_text_row;
}

__attribute__((noinline, optnone)) static void terminal_demo_scroll(void) {
  uint32_t line;

  for (line = 0u; line < DEMO_LINE_COUNT; ++line) {
    terminal_write_line_with_number(DEMO_PREFIX, line);
  }
}

static char terminal_expected_char(uint32_t row, uint32_t column) {
  if (column < sizeof(DEMO_PREFIX) - 1u) {
    return DEMO_PREFIX[column];
  }

  if (column == 14u || column == 15u) {
    const uint32_t value = row + 5u;
    return DEMO_LINE_DIGITS[value * 2u + (column - 14u)];
  }

  return ' ';
}

__attribute__((noinline, optnone)) static int terminal_verify_scroll(void) {
  uint32_t row;
  uint32_t column;

  for (row = 0u; row < CONSOLE_ROWS; ++row) {
    for (column = 0u; column < CONSOLE_COLS; ++column) {
      if (g_shadow[row][column] != terminal_expected_char(row, column)) {
        return 0;
      }
    }
  }

  return 1;
}

void kernel_main(uint64_t mb_magic, uint64_t mb_info_addr) {
  serial_init();

  if (!terminal_initialize(mb_magic, mb_info_addr)) {
    serial_write("BOOT_OK\n");
    serial_write("SCROLL_BAD\n");
    for (;;) {
      __asm__ volatile ("hlt");
    }
  }

  terminal_demo_scroll();

  serial_write("BOOT_OK\n");
  if (terminal_verify_scroll()) {
    serial_write("SCROLL_OK\n");
  } else {
    serial_write("SCROLL_BAD\n");
  }

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
