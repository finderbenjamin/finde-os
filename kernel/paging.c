#include "paging.h"

#include <stddef.h>
#include <stdint.h>

#include "pmm.h"

#define PAGE_SIZE 4096ull
#define PAGE_ENTRIES 512u

#define PAGE_PRESENT (1ull << 0)
#define PAGE_WRITABLE (1ull << 1)
#define PAGE_FLAGS (PAGE_PRESENT | PAGE_WRITABLE)

static uint64_t pml4[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pdpt_low[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pd_low[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pt_low[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pdpt_high[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pd_high[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint64_t pt_high[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void zero_table(uint64_t* table) {
  for (size_t i = 0; i < PAGE_ENTRIES; ++i) {
    table[i] = 0;
  }
}

static void map_table_chain(uint64_t* pml4_table,
                            uint16_t pml4_idx,
                            uint64_t* pdpt,
                            uint64_t* pd,
                            uint64_t* pt) {
  pml4_table[pml4_idx] = ((uint64_t)pdpt) | PAGE_FLAGS;
  pdpt[0] = ((uint64_t)pd) | PAGE_FLAGS;
  pd[0] = ((uint64_t)pt) | PAGE_FLAGS;
}

void paging_init(uint64_t kernel_start_phys, uint64_t kernel_end_phys) {
  (void)kernel_start_phys;

  zero_table(pml4);
  zero_table(pdpt_low);
  zero_table(pd_low);
  zero_table(pt_low);
  zero_table(pdpt_high);
  zero_table(pd_high);
  zero_table(pt_high);

  map_table_chain(pml4, 0, pdpt_low, pd_low, pt_low);

  const uint16_t high_idx = (uint16_t)((KERNEL_HIGH_BASE >> 39) & 0x1FFull);
  map_table_chain(pml4, high_idx, pdpt_high, pd_high, pt_high);

  const uint64_t map_end = (kernel_end_phys + PAGE_SIZE - 1ull) & ~(PAGE_SIZE - 1ull);
  for (uint64_t addr = 0; addr < map_end; addr += PAGE_SIZE) {
    const uint16_t entry = (uint16_t)((addr >> 12) & 0x1FFull);
    if (entry >= PAGE_ENTRIES) {
      break;
    }

    pt_low[entry] = addr | PAGE_FLAGS;
    pt_high[entry] = addr | PAGE_FLAGS;
  }

  uint64_t cr4;
  __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
  cr4 |= (1ull << 5);
  __asm__ volatile ("mov %0, %%cr4" : : "r"(cr4) : "memory");

  uint64_t efer_low;
  uint64_t efer_high;
  __asm__ volatile ("rdmsr" : "=a"(efer_low), "=d"(efer_high) : "c"(0xC0000080));
  uint64_t efer = (efer_high << 32) | efer_low;
  efer |= (1ull << 8);
  __asm__ volatile ("wrmsr" : : "c"(0xC0000080), "a"((uint32_t)efer), "d"((uint32_t)(efer >> 32)) : "memory");

  __asm__ volatile ("mov %0, %%cr3" : : "r"((uint64_t)pml4) : "memory");

  uint64_t cr0;
  __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= (1ull << 31);
  __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0) : "memory");
}


static uint16_t vmm_index(uint64_t virt_addr, uint8_t shift) {
  return (uint16_t)((virt_addr >> shift) & 0x1FFull);
}

static void vmm_zero_page(uint64_t phys_addr) {
  uint64_t* page = (uint64_t*)(uintptr_t)phys_addr;
  for (size_t i = 0; i < PAGE_ENTRIES; ++i) {
    page[i] = 0;
  }
}

static uint64_t* vmm_next_table(uint64_t* table, uint16_t index, uint64_t flags) {
  uint64_t entry = table[index];

  if ((entry & PAGE_PRESENT) != 0) {
    if ((entry & (1ull << 7)) != 0) {
      return (uint64_t*)0;
    }
    return (uint64_t*)(uintptr_t)(entry & ~0xFFFull);
  }

  const uint64_t new_page = pmm_alloc_page();
  if (new_page == 0) {
    return (uint64_t*)0;
  }

  vmm_zero_page(new_page);
  table[index] = new_page | (flags & 0xFFFull) | PAGE_PRESENT;
  return (uint64_t*)(uintptr_t)new_page;
}

int vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
  if ((virt_addr & 0xFFFull) != 0 || (phys_addr & 0xFFFull) != 0) {
    return -1;
  }

  uint64_t cr3;
  __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

  uint64_t* pml4_table = (uint64_t*)(uintptr_t)(cr3 & ~0xFFFull);
  uint64_t* pdpt = vmm_next_table(pml4_table, vmm_index(virt_addr, 39), flags);
  if (pdpt == (uint64_t*)0) {
    return -1;
  }

  uint64_t* pd = vmm_next_table(pdpt, vmm_index(virt_addr, 30), flags);
  if (pd == (uint64_t*)0) {
    return -1;
  }

  const uint16_t pd_index = vmm_index(virt_addr, 21);
  if ((pd[pd_index] & PAGE_PRESENT) != 0 && (pd[pd_index] & (1ull << 7)) != 0) {
    const uint64_t mapped_phys = (pd[pd_index] & 0xFFFFFFE00000ull) | (virt_addr & 0x1FFFFFull);
    if (mapped_phys == phys_addr) {
      __asm__ volatile ("invlpg (%0)" : : "r"((void*)(uintptr_t)virt_addr) : "memory");
      return 0;
    }
    return -1;
  }

  uint64_t* pt = vmm_next_table(pd, pd_index, flags);
  if (pt == (uint64_t*)0) {
    return -1;
  }

  pt[vmm_index(virt_addr, 12)] = phys_addr | (flags & 0xFFFull) | PAGE_PRESENT;
  __asm__ volatile ("invlpg (%0)" : : "r"((void*)(uintptr_t)virt_addr) : "memory");

  return 0;
}
