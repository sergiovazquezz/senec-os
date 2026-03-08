#ifndef MEMORY_PMM_H
#define MEMORY_PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} mm_entry_t;

void pmm_init(const mm_entry_t* entries, uint32_t count);
void* pmm_alloc_frame();
void pmm_free_frame(void* addr);

#endif
