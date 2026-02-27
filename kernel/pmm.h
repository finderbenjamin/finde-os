#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stdint.h>

#include "multiboot1.h"
#include "multiboot2.h"

void pmm_init(const multiboot2_info_t* mb_info);
void pmm_init_multiboot1(const multiboot1_info_t* mb1_info);
void pmm_reserve_range(uint64_t start, uint64_t end);
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t frame_addr);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t page_addr);

#endif
