#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

#define NUM_GDT_ENTRIES 7

typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_addr;
} __attribute__((packed)) tss_t;

typedef struct {
    uint64_t null;
    uint64_t kernel_code;
    uint64_t kernel_data;
    uint64_t user_code;
    uint64_t user_data;
    uint64_t tss_low;
    uint64_t tss_high;
} __attribute__((packed)) gdt_t;

typedef struct {
    uint16_t limit;
    uint64_t address;
} __attribute__((packed)) gdtr_t;

void gdt_init();

#endif // KERNEL_GDT_H
