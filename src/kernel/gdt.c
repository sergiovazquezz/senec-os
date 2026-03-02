#include "gdt.h"
#include <stdint.h>

static gdt_t gdt;
static gdtr_t gdtr;
static tss_t tss;

static uint64_t tss_descriptor_low(uint64_t base, uint32_t limit)
{
    const uint64_t limit_low = (limit & 0xFFFF);       // bits [15:0]
    const uint64_t base_low = (base & 0xFFFFFF) << 16; // bits [39:16]
    const uint64_t type = 0x89ULL << 40; // bits [47:40] P=1, type=0x9
    const uint64_t limit_high = ((limit >> 16) & 0xFULL) << 48; // bits [51:48]
    const uint64_t base_high = ((base >> 24) & 0xFF) << 56;     // bits [63:56]

    return limit_low | base_low | type | limit_high | base_high;
}

static uint64_t tss_descriptor_high(uint64_t base)
{
    return (base >> 32) & 0xFFFFFFFF;
}

static void gdt_create()
{
    gdt.null = 0;

    gdt.kernel_code = (0xAULL << 52) | (0x9AULL << 40);
    gdt.kernel_data = (0x8ULL << 52) | (0x92ULL << 40);

    gdt.user_code = (0xAULL << 52) | (0xFAULL << 40);
    gdt.user_data = (0x8ULL << 52) | (0xF2ULL << 40);

    const uint64_t base = (uint64_t)&tss;
    const uint32_t limit = sizeof(tss_t) - 1;

    gdt.tss_low = tss_descriptor_low(base, limit);
    gdt.tss_high = tss_descriptor_high(base);
}

static inline void gdt_load() { asm("lgdt %0" : : "m"(gdtr)); }

static inline void gdt_reload_segments()
{
    asm("push %0\n"
        "lea rax, [rip + 1f]\n"
        "push rax\n"
        "retfq\n"
        "1:\n"
        :
        : "i"(0x08)
        : "rax", "memory");

    asm("mov ds, %0\n"
        "mov es, %0\n"
        "mov fs, %0\n"
        "mov gs, %0\n"
        "mov ss, %0\n"
        :
        : "r"((uint16_t)0x10));
}

static inline void tss_load() { asm("ltr %0" : : "r"((uint16_t)0x28)); }

void gdt_init()
{
    gdt_create();

    gdtr.limit = sizeof(gdt_t) - 1;
    gdtr.address = (uint64_t)&gdt;

    gdt_load();
    gdt_reload_segments();
    tss_load();
}
