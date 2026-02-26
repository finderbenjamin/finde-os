#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stdint.h>

#include "multiboot1.h"
#include "multiboot2.h"

void pmm_init(const multiboot2_info_t* mb_info);
void pmm_init_multiboot1(const multiboot1_info_t* mb1_info);
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t frame_addr);

#endif
