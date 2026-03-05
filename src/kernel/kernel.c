#include "kernel.h"

#include "../boot/helpers.h"
#include "../boot/multiboot2.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../drivers/printk.h"

#include <stdbool.h>
#include <stdint.h>

void kmain(uint32_t magic, void* mboot_info)
{
    printk_init();

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        printk("\n[OK] Magic number correct\n");
    } else {
        printk("\n[FAIL] Magic number not correct\n");
    }

    printk("Hello from the kernel!\n");

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        parse_multiboot2(mboot_info);
    }

    tss_init();

    gdt_init();

    idt_init();

    asm volatile("sti");

    while (true)
        asm volatile("hlt");
}
