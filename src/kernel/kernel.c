#include "kernel.h"

#include "../boot/helpers.h"
#include "../boot/multiboot2.h"
#include "../cpu/acpi.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../drivers/printk.h"
#include "../memory/pmm.h"

#include <stdbool.h>
#include <stdint.h>

void kmain(uint32_t magic, void* mboot_info)
{
    printk_init();

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC)
        printk("\n[OK] Magic number correct\n");
    else
        printk("\n[FAIL] Magic number not correct\n");

    printk("Hello from the kernel!\n");

    multiboot2_info_t mb2 = { 0 };
    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC)
        mb2 = parse_multiboot2(mboot_info);

    pmm_init(mb2.mmap, mb2.mmap_count);

    acpi_init(mb2.rsdp);

    tss_init();

    gdt_init();

    idt_init();

    pic_disable();

    asm volatile("sti");

    while (true)
        asm volatile("hlt");
}
