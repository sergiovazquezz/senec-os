#include "pmm.h"

#include "../boot/constants.h"
#include "../boot/multiboot2.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t* bitmap;
static uint64_t total_frames;

static inline void bitmap_set(uint64_t frame)
{
    bitmap[frame / 8] |= (uint8_t)(1 << (frame % 8));
}

static inline void bitmap_clear(uint64_t frame)
{
    bitmap[frame / 8] &= (uint8_t)~(1 << (frame % 8));
}

static inline bool bitmap_is_frame_used(uint64_t frame)
{
    return (bitmap[frame / 8] >> (frame % 8)) & 1;
}

extern uint8_t __kernel_end;
static uintptr_t bump;

static void* bump_alloc(uint64_t bytes)
{
    void* ptr = (void*)bump;
    bump += bytes;
    return ptr;
}

void pmm_init(const mm_entry_t entries[], uint32_t count)
{
    uintptr_t max_addr = 0;

    for (uint32_t i = 0; i < count; i++) {
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE)
            continue;

        const uint64_t end = entries[i].addr + entries[i].len;

        if (end > max_addr)
            max_addr = end;
    }

    total_frames = (max_addr + PAGE_SIZE - 1) / PAGE_SIZE;
    const uint64_t bitmap_bytes = (total_frames + 7) / 8;

    bump = (uintptr_t)&__kernel_end;
    bump = (bump + PAGE_SIZE - 1) & ~(uintptr_t)(PAGE_SIZE - 1);

    bitmap = (uint8_t*)P2V(bump_alloc(bitmap_bytes));

    for (size_t i = 0; i < bitmap_bytes; i++)
        bitmap[i] = 0xFF;

    for (uint32_t i = 0; i < count; i++) {
        if (entries[i].type != MULTIBOOT_MEMORY_AVAILABLE)
            continue;

        const uintptr_t start = entries[i].addr;
        const uintptr_t end = entries[i].addr + entries[i].len;

        const uint64_t first_frame = (start + PAGE_SIZE - 1) / PAGE_SIZE;
        const uint64_t last_frame = end / PAGE_SIZE;

        for (uint64_t f = first_frame; f < last_frame; f++)
            bitmap_clear(f);
    }

    const uint64_t kernel_first_frame = 0x100000 / PAGE_SIZE;
    const uint64_t kernel_last_frame = (bump + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t f = kernel_first_frame; f < kernel_last_frame; f++)
        bitmap_set(f);

    bitmap_set(0);
}

void* pmm_alloc_frame()
{
    for (size_t f = 1; f < total_frames; f++) {
        if (!bitmap_is_frame_used(f)) {
            bitmap_set(f);
            return (void*)(uintptr_t)(f * PAGE_SIZE);
        }
    }

    return NULL;
}

void pmm_free_frame(void* addr)
{
    const uint64_t frame = (uintptr_t)addr / PAGE_SIZE;

    if (frame < total_frames)
        bitmap_clear(frame);
}
