#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include <stdint.h>

#define KERNEL_HIGH_BASE 0xFFFFFFFF80000000ull

#define VMM_PAGE_PRESENT (1ull << 0)
#define VMM_PAGE_WRITABLE (1ull << 1)

void paging_init(uint64_t kernel_start_phys, uint64_t kernel_end_phys);
int vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

#endif
