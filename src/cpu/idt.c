#include "idt.h"
#include "../drivers/printk.h"
#include "cpu.h"

#include <stddef.h>
#include <stdint.h>

static idt_t idt[IDT_SIZE];

extern const uintptr_t interrupt_handlers[IDT_NUM_HANDLERS];

static void idt_set_entry(uint8_t vector, uintptr_t handler_addr, uint8_t dpl)
{
    idt_t* entry = &idt[vector];

    entry->address_low = handler_addr & 0xFFFF;
    entry->address_mid = (handler_addr >> 16) & 0xFFFF;
    entry->address_high = handler_addr >> 32;

    entry->selector = 0x08;

    entry->flags = INTERRUPT_GATE | ((dpl & 0b11) << 5) | IDT_ENTRY_FLAG_PRESENT;

    entry->ist = 0;
}

void idt_init()
{
    for (size_t i = 0; i < IDT_NUM_HANDLERS; i++)
        idt_set_entry(i, interrupt_handlers[i], 0);

    const idtr_t idtr = { .limit = sizeof(idt) - 1, .address = (uint64_t)idt };

    asm volatile("lidt %0" : : "m"(idtr));
}

static const char* exception_names[32] = { "Divide by Zero Error",
                                           "Debug",
                                           "Non Maskable Interrupt",
                                           "Breakpoint",
                                           "Overflow",
                                           "Bound Range",
                                           "Invalid Opcode",
                                           "Device Not Available",
                                           "Double Fault",
                                           "Coprocessor Segment Overrun",
                                           "Invalid TSS",
                                           "Segment Not Present",
                                           "Stack-Segment Fault",
                                           "General Protection Fault",
                                           "Page Fault",
                                           "Reserved",
                                           "x87 Floating-Point Exception",
                                           "Alignment Check",
                                           "Machine Check",
                                           "SIMD Floating-Point Exception",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Reserved",
                                           "Security Exception",
                                           "Reserved" };

void interrupt_dispatch(cpu_status_t* cpu_status)
{
    uint64_t vec_num = cpu_status->vector_number;

    const char* vec_name = (vec_num < 32) ? exception_names[vec_num] : "Unknown";

    printk("\n=== EXCEPTION ===\n");
    printk("Vector: %d - %s\n", vec_num, vec_name);

    printk("Error: %p\n", cpu_status->error_code);

    printk("RIP: %p\n", cpu_status->iret_rip);

    printk("RSP: %p\n", cpu_status->iret_rsp);

    printk("=================\n");
    printk("Halting.\n");

    for (;;)
        asm volatile("hlt");
}
