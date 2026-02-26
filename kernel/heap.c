#include "heap.h"

#include <stdint.h>

#define HEAP_SIZE (64 * 1024)
#define HEAP_ALIGNMENT 16

static uint8_t g_heap[HEAP_SIZE] __attribute__((aligned(HEAP_ALIGNMENT)));
static size_t g_heap_offset = 0;

static size_t align_up_size(size_t value, size_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

void heap_init(void) {
  g_heap_offset = 0;
}

void* kmalloc(size_t size) {
  if (size == 0) {
    return (void*)0;
  }

  size_t aligned_offset = align_up_size(g_heap_offset, HEAP_ALIGNMENT);
  size_t aligned_size = align_up_size(size, HEAP_ALIGNMENT);

  if (aligned_offset > HEAP_SIZE || aligned_size > HEAP_SIZE - aligned_offset) {
    return (void*)0;
  }

  g_heap_offset = aligned_offset + aligned_size;
  return (void*)&g_heap[aligned_offset];
}

void kfree(void* p) {
  (void)p;
  /* no-op for bump allocator */
}
