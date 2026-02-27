#include "pmm.h"

#include <stddef.h>
#include <stdint.h>

#define PMM_FRAME_SIZE 4096u
#define PMM_MAX_FRAMES 131072u
#define PMM_BITMAP_WORDS (PMM_MAX_FRAMES / 64u)

static uint64_t frame_bitmap[PMM_BITMAP_WORDS];

static inline void bitmap_set(uint64_t frame_index) {
  frame_bitmap[frame_index / 64u] |= (1ull << (frame_index % 64u));
}

static inline void bitmap_clear(uint64_t frame_index) {
  frame_bitmap[frame_index / 64u] &= ~(1ull << (frame_index % 64u));
}

static inline int bitmap_test(uint64_t frame_index) {
  return (frame_bitmap[frame_index / 64u] & (1ull << (frame_index % 64u))) != 0;
}

static uint64_t pmm_align_down(uint64_t value) {
  return value & ~(uint64_t)(PMM_FRAME_SIZE - 1u);
}

static uint64_t pmm_align_up(uint64_t value) {
  return (value + PMM_FRAME_SIZE - 1u) & ~(uint64_t)(PMM_FRAME_SIZE - 1u);
}

static void pmm_mark_usable_range(uint64_t start, uint64_t end) {
  if (end > (uint64_t)PMM_MAX_FRAMES * PMM_FRAME_SIZE) {
    end = (uint64_t)PMM_MAX_FRAMES * PMM_FRAME_SIZE;
  }

  if (start >= end) {
    return;
  }

  uint64_t frame = (start + PMM_FRAME_SIZE - 1u) / PMM_FRAME_SIZE;
  const uint64_t frame_end = end / PMM_FRAME_SIZE;

  while (frame < frame_end) {
    bitmap_clear(frame);
    ++frame;
  }
}

static void pmm_reserve_low_memory(void) {
  for (uint64_t frame = 0; frame < 256u; ++frame) {
    bitmap_set(frame);
  }
}

void pmm_init(const multiboot2_info_t* mb_info) {
  for (size_t i = 0; i < PMM_BITMAP_WORDS; ++i) {
    frame_bitmap[i] = ~0ull;
  }

  if (mb_info != (const multiboot2_info_t*)0) {
    const uint8_t* info_end = (const uint8_t*)mb_info + mb_info->total_size;
    const multiboot2_tag_t* tag = multiboot2_first_tag(mb_info);

    size_t tag_count = 0;
    while ((const uint8_t*)tag + sizeof(multiboot2_tag_t) <= info_end && tag_count < 256u) {
      if (tag->type == MULTIBOOT2_TAG_TYPE_END) {
        break;
      }

      if (tag->size < sizeof(multiboot2_tag_t)) {
        break;
      }

      if (tag->type == MULTIBOOT2_TAG_TYPE_MMAP) {
        const multiboot2_tag_mmap_t* mmap_tag = (const multiboot2_tag_mmap_t*)tag;
        const uint8_t* entries_end = (const uint8_t*)mmap_tag + mmap_tag->size;
        const uint8_t* entry_ptr = (const uint8_t*)mmap_tag + sizeof(multiboot2_tag_mmap_t);

        if (mmap_tag->entry_size < sizeof(multiboot2_mmap_entry_t)) {
          tag = multiboot2_next_tag(tag);
          continue;
        }

        if (entries_end > info_end) {
          entries_end = info_end;
        }

        while (entry_ptr + mmap_tag->entry_size <= entries_end) {
          const multiboot2_mmap_entry_t* entry = (const multiboot2_mmap_entry_t*)entry_ptr;

          if (entry->type == MULTIBOOT2_MEMORY_AVAILABLE) {
            pmm_mark_usable_range(entry->addr, entry->addr + entry->len);
          }

          entry_ptr += mmap_tag->entry_size;
        }
      }

      const multiboot2_tag_t* next_tag = multiboot2_next_tag(tag);
      if ((const uint8_t*)next_tag <= (const uint8_t*)tag) {
        break;
      }

      tag = next_tag;
      ++tag_count;
    }
  } else {
    pmm_mark_usable_range(0x100000ull, 0x4000000ull);
  }

  pmm_reserve_low_memory();
}

void pmm_init_multiboot1(const multiboot1_info_t* mb1_info) {
  for (size_t i = 0; i < PMM_BITMAP_WORDS; ++i) {
    frame_bitmap[i] = ~0ull;
  }

  if (mb1_info != (const multiboot1_info_t*)0 && (mb1_info->flags & MULTIBOOT1_FLAG_MMAP) != 0) {
    const uint8_t* ptr = (const uint8_t*)(uintptr_t)mb1_info->mmap_addr;
    const uint8_t* end = ptr + mb1_info->mmap_length;

    while (ptr + sizeof(uint32_t) <= end) {
      const multiboot1_mmap_entry_t* entry = (const multiboot1_mmap_entry_t*)ptr;
      if (entry->type == MULTIBOOT1_MEMORY_AVAILABLE) {
        pmm_mark_usable_range(entry->addr, entry->addr + entry->len);
      }
      ptr += entry->size + sizeof(uint32_t);
    }
  }

  pmm_reserve_low_memory();
}

void pmm_reserve_range(uint64_t start, uint64_t end) {
  const uint64_t max_addr = (uint64_t)PMM_MAX_FRAMES * PMM_FRAME_SIZE;

  if (start >= max_addr) {
    return;
  }

  if (end > max_addr) {
    end = max_addr;
  }

  if (start >= end) {
    return;
  }

  uint64_t frame = pmm_align_down(start) / PMM_FRAME_SIZE;
  const uint64_t frame_end = pmm_align_up(end) / PMM_FRAME_SIZE;

  while (frame < frame_end) {
    bitmap_set(frame);
    ++frame;
  }
}

uint64_t pmm_alloc_frame(void) {
  for (uint64_t i = 0; i < PMM_MAX_FRAMES; ++i) {
    if (!bitmap_test(i)) {
      bitmap_set(i);
      return i * PMM_FRAME_SIZE;
    }
  }

  return 0;
}

void pmm_free_frame(uint64_t frame_addr) {
  const uint64_t frame = frame_addr / PMM_FRAME_SIZE;
  if (frame >= PMM_MAX_FRAMES) {
    return;
  }

  bitmap_clear(frame);
}
