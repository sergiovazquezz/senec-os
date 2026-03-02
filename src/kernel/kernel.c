#include "kernel.h"

#include "../drivers/serial.h"
#include "../drivers/terminal.h"

#include "../boot/helpers.h"
#include "../boot/multiboot2.h"

#include <stdbool.h>
#include <stdint.h>

void kmain(uint32_t magic, void* mboot_info)
{
    serial_init();
    terminal_init();

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        serial_puts("\n[OK] Magic number correct\n");
    } else {
        serial_puts("\n[FAIL] Magic number not correct\n");
    }

    terminal_write_string("Hello from the kernel!\n");

    serial_puts("x86_64 higher-half kernel running at 0xFFFFFFFF80000000\n");

    if (magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
        parse_multiboot2(mboot_info);
    }

    while (true) {
        asm volatile("hlt");
    }
}
