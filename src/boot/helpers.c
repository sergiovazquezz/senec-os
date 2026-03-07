#include "helpers.h"

#include "../drivers/printk.h"

#include <stddef.h>

rsdp_t parse_multiboot2(void* mbi)
{
    uint32_t total_size = *(uint32_t*)mbi;
    printk("[MB2] total_size = %d bytes\n", total_size);

    struct multiboot_tag* tag = mb2_first_tag(mbi);

    rsdp_t rsdp = { .version = RSDP_VERSION_NONE };

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        switch (tag->type) {

        case MULTIBOOT_TAG_TYPE_CMDLINE: {
            struct multiboot_tag_string* t = (struct multiboot_tag_string*)tag;
            printk("[MB2] cmdline: %s\n", t->string);
            break;
        }

        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
            struct multiboot_tag_string* t = (struct multiboot_tag_string*)tag;
            printk("[MB2] bootloader: %s\n", t->string);
            break;
        }

        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
            struct multiboot_tag_basic_meminfo* t = (struct multiboot_tag_basic_meminfo*)tag;
            printk("[MB2] mem_lower = %d KB, mem_upper = %d KB\n", t->mem_lower, t->mem_upper);
            break;
        }

        case MULTIBOOT_TAG_TYPE_MMAP: {
            struct multiboot_tag_mmap* t = (struct multiboot_tag_mmap*)tag;
            printk("[MB2] memory map:\n");

            uint32_t n_entries = (t->size - sizeof(*t)) / t->entry_size;

            for (uint32_t i = 0; i < n_entries; i++) {
                struct multiboot_mmap_entry* e =
                    (struct multiboot_mmap_entry*)((uint8_t*)t->entries + i * t->entry_size);

                printk(" %p - %p len %p (%s)\n", e->addr, (e->addr + e->len), e->len, mmap_type_str(e->type));
            }

            break;
        }

        case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
            struct multiboot_tag_new_acpi* t = (struct multiboot_tag_new_acpi*)tag;
            printk("[MB2] ACPI 2.0 RSDP found\n");

            rsdp.version = RSDP_VERSION_2;
            rsdp.v2 = (rsdp2_t*)t->rsdp;

            break;
        }

        case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
            struct multiboot_tag_old_acpi* t = (struct multiboot_tag_old_acpi*)tag;
            printk("[MB2] ACPI 1.0 RSDP found\n");

            if (rsdp.version == RSDP_VERSION_NONE) {
                rsdp.version = RSDP_VERSION_1;
                rsdp.v1 = (rsdp1_t*)t->rsdp;
            }

            break;
        }

        default:
            break;
        }

        tag = mb2_next_tag(tag);
    }

    return rsdp;
}
