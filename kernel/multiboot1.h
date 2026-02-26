#ifndef KERNEL_MULTIBOOT1_H
#define KERNEL_MULTIBOOT1_H

#include <stdint.h>

#define MULTIBOOT1_BOOTLOADER_MAGIC 0x2BADB002u
#define MULTIBOOT1_FLAG_MMAP (1u << 6)
#define MULTIBOOT1_MEMORY_AVAILABLE 1u

typedef struct __attribute__((packed)) {
  uint32_t size;
  uint64_t addr;
  uint64_t len;
  uint32_t type;
} multiboot1_mmap_entry_t;

typedef struct __attribute__((packed)) {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms[4];
  uint32_t mmap_length;
  uint32_t mmap_addr;
} multiboot1_info_t;

#endif
