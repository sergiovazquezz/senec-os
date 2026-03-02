#include "helpers.h"

#include "../drivers/serial.h"

void parse_multiboot2(void* mbi)
{
    // The first 4 bytes of the info structure are total_size
    uint32_t total_size = *(uint32_t*)mbi;
    serial_puts("[MB2] total_size = ");
    serial_putd(total_size);
    serial_puts(" bytes\n");

    struct multiboot_tag* tag = mb2_first_tag(mbi);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {

        case MULTIBOOT_TAG_TYPE_CMDLINE: {
            struct multiboot_tag_string* t = (struct multiboot_tag_string*)tag;
            serial_puts("[MB2] cmdline: ");
            serial_puts(t->string);
            serial_puts("\n");
            break;
        }

        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
            struct multiboot_tag_string* t = (struct multiboot_tag_string*)tag;
            serial_puts("[MB2] bootloader: ");
            serial_puts(t->string);
            serial_puts("\n");
            break;
        }

        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
            struct multiboot_tag_basic_meminfo* t =
                (struct multiboot_tag_basic_meminfo*)tag;
            serial_puts("[MB2] mem_lower = ");
            serial_putd(t->mem_lower);
            serial_puts(" KB, mem_upper = ");
            serial_putd(t->mem_upper);
            serial_puts(" KB\n");
            break;
        }

        case MULTIBOOT_TAG_TYPE_MMAP: {
            struct multiboot_tag_mmap* t = (struct multiboot_tag_mmap*)tag;
            serial_puts("[MB2] memory map:\n");

            uint32_t n_entries = (t->size - sizeof(*t)) / t->entry_size;

            for (uint32_t i = 0; i < n_entries; i++) {
                struct multiboot_mmap_entry* e =
                    (struct multiboot_mmap_entry*)((uint8_t*)t->entries +
                                                   i * t->entry_size);
                serial_puts("  ");
                serial_puth(e->addr);
                serial_puts(" - ");
                serial_puth(e->addr + e->len);
                serial_puts("  len ");
                serial_puth(e->len);
                serial_puts("  (");
                serial_puts(mmap_type_str(e->type));
                serial_puts(")\n");
            }
            break;
        }

        default:
            break;
        }

        tag = mb2_next_tag(tag);
    }
}
