#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

__attribute__((noreturn)) void panic(const char* msg);
__attribute__((noreturn)) void panic_assert(const char* file, int line);

#define ASSERT(x) do { \
  if (!(x)) { \
    panic_assert(__FILE__, __LINE__); \
  } \
} while (0)

#endif
