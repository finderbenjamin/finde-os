#ifndef KERNEL_MULTIBOOT2_H
#define KERNEL_MULTIBOOT2_H

#include <stddef.h>
#include <stdint.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36D76289u
#define MULTIBOOT2_TAG_TYPE_END 0u
#define MULTIBOOT2_TAG_TYPE_MMAP 6u
#define MULTIBOOT2_MEMORY_AVAILABLE 1u

typedef struct {
  uint32_t type;
  uint32_t size;
} multiboot2_tag_t;

typedef struct {
  uint32_t total_size;
  uint32_t reserved;
} multiboot2_info_t;

typedef struct {
  uint32_t type;
  uint32_t size;
  uint32_t entry_size;
  uint32_t entry_version;
} multiboot2_tag_mmap_t;

typedef struct {
  uint64_t addr;
  uint64_t len;
  uint32_t type;
  uint32_t zero;
} multiboot2_mmap_entry_t;

static inline const multiboot2_tag_t* multiboot2_first_tag(const multiboot2_info_t* info) {
  return (const multiboot2_tag_t*)((const uint8_t*)info + 8u);
}

static inline const multiboot2_tag_t* multiboot2_next_tag(const multiboot2_tag_t* tag) {
  const uintptr_t next = ((uintptr_t)tag + (uintptr_t)tag->size + 7u) & ~(uintptr_t)7u;
  return (const multiboot2_tag_t*)next;
}

#endif
