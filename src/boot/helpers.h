#ifndef MULTIBOOT_HELPERS_H
#define MULTIBOOT_HELPERS_H

#include "multiboot2.h"
#include <stdint.h>

void parse_multiboot2(void* mbi);

static inline struct multiboot_tag* mb2_first_tag(void* mbi)
{
    return (struct multiboot_tag*)((uint8_t*)mbi + 8);
}

static inline struct multiboot_tag* mb2_next_tag(struct multiboot_tag* tag)
{
    uint32_t aligned = (tag->size + 7) & ~7u;
    return (struct multiboot_tag*)((uint8_t*)tag + aligned);
}

static inline const char* mmap_type_str(uint32_t type)
{
    switch (type) {
    case MULTIBOOT_MEMORY_AVAILABLE:
        return "available";
    case MULTIBOOT_MEMORY_RESERVED:
        return "reserved";
    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
        return "ACPI reclaimable";
    case MULTIBOOT_MEMORY_NVS:
        return "NVS";
    case MULTIBOOT_MEMORY_BADRAM:
        return "bad RAM";
    default:
        return "unknown";
    }
}

#endif // MULTIBOOT_HELPERS_H
