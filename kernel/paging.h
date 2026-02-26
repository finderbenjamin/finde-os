#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>

#define KERNEL_HIGH_BASE 0xFFFFFFFF80000000ull

void paging_init(uint64_t kernel_start_phys, uint64_t kernel_end_phys);

#endif
