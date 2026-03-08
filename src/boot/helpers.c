#include "helpers.h"

#include "../drivers/printk.h"

#include <stddef.h>
#include <stdint.h>

multiboot2_info_t parse_multiboot2(void* mbi)
{
    uint32_t total_size = *(uint32_t*)mbi;
    printk("[MB2] total_size = %d bytes\n", total_size);

    struct multiboot_tag* tag = mb2_first_tag(mbi);

    multiboot2_info_t info = {
        .rsdp = { .version = RSDP_VERSION_NONE },
        .mmap_count = 0,
    };

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

            uint32_t num_entries = (t->size - sizeof(*t)) / t->entry_size;

            for (uint32_t i = 0; i < num_entries; i++) {
                struct multiboot_mmap_entry* entry =
                    (struct multiboot_mmap_entry*)((uint8_t*)t->entries + i * t->entry_size);

                if (info.mmap_count < MMAP_MAX_ENTRIES) {
                    info.mmap[info.mmap_count].addr = entry->addr;
                    info.mmap[info.mmap_count].len = entry->len;
                    info.mmap[info.mmap_count].type = entry->type;
                    info.mmap_count++;
                }
            }

            break;
        }
            //
            // case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
            //     struct multiboot_tag_new_acpi* t = (struct multiboot_tag_new_acpi*)tag;
            //     printk("[MB2] ACPI 2.0 RSDP found\n");
            //
            //     info.rsdp.version = RSDP_VERSION_2;
            //     info.rsdp.v2 = (rsdp2_t*)t->rsdp;
            //
            //     break;
            // }
            //
            // case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
            //     struct multiboot_tag_old_acpi* t = (struct multiboot_tag_old_acpi*)tag;
            //     printk("[MB2] ACPI 1.0 RSDP found\n");
            //
            //     if (info.rsdp.version == RSDP_VERSION_NONE) {
            //         info.rsdp.version = RSDP_VERSION_1;
            //         info.rsdp.v1 = (rsdp1_t*)t->rsdp;
            //     }
            //
            //     break;
            // }

        default:
            break;
        }

        tag = mb2_next_tag(tag);
    }

    return info;
}
