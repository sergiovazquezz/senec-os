#ifndef MEMORY_VMM_H
#define MEMORY_VMM_H

#include "../boot/constants.h"
#include "pmm.h"

#include <stddef.h>
#include <stdint.h>

#define PTE_PRESENT           (1ULL << 0)
#define PTE_WRITABLE          (1ULL << 1)
#define PTE_USER              (1ULL << 2)
#define PTE_PWT               (1ULL << 3)
#define PTE_PCD               (1ULL << 4)
#define PTE_ACCESSED          (1ULL << 5)
#define PTE_DIRTY             (1ULL << 6)
#define PTE_HUGE              (1ULL << 7)
#define PTE_GLOBAL            (1ULL << 8)
#define PTE_NX                (1ULL << 63)

#define PTE_ADDR_MASK         0x000FFFFFFFFFF000ULL

#define VMM_FLAGS_KERNEL_RW   (PTE_PRESENT | PTE_WRITABLE | PTE_NX)
#define VMM_FLAGS_KERNEL_RO   (PTE_PRESENT | PTE_NX)
#define VMM_FLAGS_KERNEL_CODE (PTE_PRESENT)
#define VMM_FLAGS_MMIO        (PTE_PRESENT | PTE_WRITABLE | PTE_NX | PTE_PCD | PTE_PWT)

#define PML4_INDEX(va)        (((va) >> 39) & 0x1FF)
#define PDPT_INDEX(va)        (((va) >> 30) & 0x1FF)
#define PD_INDEX(va)          (((va) >> 21) & 0x1FF)
#define PT_INDEX(va)          (((va) >> 12) & 0x1FF)

static inline void vmm_invlpg(uintptr_t virt_addr)
{
    asm volatile("invlpg [%0]" : : "r"(virt_addr) : "memory");
}

// TLB flush (reload CR3)
static inline void vmm_flush_tlb(void)
{
    uintptr_t cr3;
    asm volatile("mov %0, cr3" : "=r"(cr3));
    asm volatile("mov cr3, %0" : : "r"(cr3) : "memory");
}

// Returns the physical address of the current PML4
static inline uintptr_t vmm_read_cr3(void)
{
    uintptr_t cr3;
    asm volatile("mov %0, cr3" : "=r"(cr3));
    return cr3 & PTE_ADDR_MASK;
}

void vmm_init(mm_entry_t mmap_entries[], uint32_t num_entries);
int8_t vmm_map_page(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);
void vmm_unmap_page(uintptr_t virt_addr);
void* vmm_map_mmio(uintptr_t phys_addr, size_t size);

#endif
