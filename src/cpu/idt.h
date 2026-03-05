#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

// Clears interrupt flag before running handler (disables nested interrupts)
#define INTERRUPT_GATE         0b1110
// Does not clear interrupt flag (nested interrupts can happen)
#define TRAP_GATE              0b1111

#define IDT_SIZE               256
#define IDT_ENTRY_FLAG_PRESENT (1 << 7)

#define IDT_NUM_HANDLERS       32

typedef struct {
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__((packed)) idt_t;

typedef struct {
    uint16_t limit;
    uint64_t address;
} __attribute__((packed)) idtr_t;

void idt_init();

#endif // KERNEL_IDT_H
