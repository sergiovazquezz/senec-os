#include "vmm.h"

#include "../boot/constants.h"
#include "../drivers/printk.h"
#include "../string/string.h"
#include "pmm.h"

#include <stddef.h>
#include <stdint.h>

static uint64_t* get_pml4(void)
{
    return (uint64_t*)P2V_LATE(vmm_read_cr3());
}

static uint64_t* pte_to_table(uintptr_t entry)
{
    return (uint64_t*)P2V_LATE(entry & PTE_ADDR_MASK);
}

void vmm_init(mm_entry_t mmap_entries[], uint32_t num_entries)
{
    uintptr_t max_phys_addr = 0;

    for (uint32_t i = 0; i < num_entries; i++) {
        const uint64_t end = mmap_entries[i].addr + mmap_entries[i].len;

        if (end > max_phys_addr)
            max_phys_addr = end;
    }

    // Round up to next 2MB
    max_phys_addr = (max_phys_addr + (1ULL << 21) - 1) & ~((1ULL << 21) - 1);

    uint64_t* pml4 = get_pml4();

    for (uintptr_t phys_addr = 0; phys_addr < max_phys_addr; phys_addr += (1ULL << 30)) {
        uintptr_t virt_addr = P2V_LATE(phys_addr);

        uint16_t pml4_idx = PML4_INDEX(virt_addr);
        uint16_t pdpt_idx = PDPT_INDEX(virt_addr);

        if (!(pml4[pml4_idx] & PTE_PRESENT)) {
            void* frame = pmm_alloc_frame();

            // TODO: Handle this case in some better form
            if (!frame) {
                printk("[VMM] FATAL: out of memory mapping (direct map)\n");
                return;
            }

            // WARN: Is this not supposed to be a huge page?
            memset((void*)P2V_LATE(frame), 0, PAGE_SIZE);
            pml4[pml4_idx] = (uintptr_t)frame | PTE_PRESENT | PTE_WRITABLE;
        }

        uint64_t* pdpt = pte_to_table(pml4[pml4_idx]);

        if (pdpt[pdpt_idx] & PTE_PRESENT)
            continue;

        void* pd_frame = pmm_alloc_frame();

        // NOTE: Maybe replace with goto?
        if (!pd_frame) {
            printk("[VMM] FATAL: out of memory mapping (direct map)\n");
            return;
        }

        uint64_t* pd = (uint64_t*)P2V_LATE(pd_frame);

        for (uint16_t i = 0; i < 256; i++) {
            uintptr_t frame_addr = phys_addr + (uintptr_t)i * (1ULL << 21);
            pd[i] = frame_addr | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE | PTE_NX;
        }
    }

    vmm_flush_tlb();

    printk("[VMM] Direct map complete\n");
}

int8_t vmm_map_page(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);
void vmm_unmap_page(uintptr_t virt_addr);
void* vmm_map_mmio(uintptr_t phys_addr, size_t size);
