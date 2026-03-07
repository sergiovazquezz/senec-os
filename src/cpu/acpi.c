#include "acpi.h"

#include "../boot/constants.h"
#include "../drivers/printk.h"
#include "../drivers/serial.h"
#include "../string/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PIC_COMMAND_MASTER 0x20
#define PIC_DATA_MASTER    0x21
#define PIC_COMMAND_SLAVE  0xA0
#define PIC_DATA_SLAVE     0xA1

#define ICW_1              0x11
#define ICW_2_M            0x20
#define ICW_2_S            0x28
#define ICW_3_M            0x04
#define ICW_3_S            0x02
#define ICW_4              0x01

// Validate checksum
static bool acpi_validate(char* byte_array, size_t length)
{
    uint32_t sum = 0;

    for (size_t i = 0; i < length; i++)
        sum += byte_array[i];

    return (sum & 0xFF) == 0;
}

// Check signature and validate checksum of a candidate table
static acpi_sdt_header_t* check_table(acpi_sdt_header_t* h, const char sig[4])
{
    if (memcmp(h->signature, sig, 4))
        return NULL;

    if (!acpi_validate((char*)h, h->length)) {
        printk("[ACPI] WARNING: table '%c %c %c %c' failed checksum", sig[0], sig[1], sig[2], sig[3]);

        return NULL;
    }

    return h;
}

// Search RSDT for a header with a specific signature
static acpi_sdt_header_t* acpi_find_table_rsdt(rsdt_t* r, size_t num_items, const char sig[4])
{
    for (size_t i = 0; i < num_items; i++) {
        acpi_sdt_header_t* h = (acpi_sdt_header_t*)P2V(r->entries[i]);

        acpi_sdt_header_t* result = check_table(h, sig);

        if (result)
            return result;
    }

    return NULL;
}

// Search XSDT for a header with a specific signature
static acpi_sdt_header_t* acpi_find_table_xsdt(xsdt_t* x, size_t num_items, const char sig[4])
{
    for (size_t i = 0; i < num_items; i++) {
        acpi_sdt_header_t* h = (acpi_sdt_header_t*)P2V(x->entries[i]);

        void* result = check_table(h, sig);

        if (result)
            return result;
    }

    return NULL;
}

// Walk MADT entries, log LAPIC and I/O APIC info.
static void parse_madt(madt_t* madt)
{
    printk("[ACPI] MADT: LAPIC base = %p, flags = %p\n", madt->lapic_address, madt->flags);

    uint8_t* entry = (uint8_t*)(madt + 1);
    uint8_t* end = (uint8_t*)madt + madt->header.length;

    while (entry < end) {
        madt_entry_header_t* hdr = (madt_entry_header_t*)entry;

        switch (hdr->type) {
        case 0: {
            madt_entry_lapic_t* lapic = (madt_entry_lapic_t*)entry;
            printk("[ACPI] LAPIC: processor_id=%d apic_id=%d flags=%d\n", lapic->acpi_processor_id,
                   lapic->apic_id, lapic->flags);
            break;
        }
        case 1: {
            madt_entry_ioapic_t* ioapic = (madt_entry_ioapic_t*)entry;
            printk("[ACPI] IOAPIC: id=%d addr=%d gsi_base=%d\n", ioapic->ioapic_id, ioapic->ioapic_address,
                   ioapic->gsi_base);
            break;
        }
        case 2: {
            madt_entry_iso_t* iso = (madt_entry_iso_t*)entry;
            printk("[ACPI] ISO: bus=%d irq=%d -> gsi=%d flags=%p\n", iso->bus, iso->gsi, iso->flags);
            break;
        }
        default:
            printk("[ACPI] MADT entry type=%d (skipped)\n", hdr->type);
            break;
        }

        entry += hdr->length;
    }
}

void acpi_init(rsdp_t rsdp)
{
    if (rsdp.version == RSDP_VERSION_NONE) {
        printk("[ACPI] No RSDP provided\n");
        return;
    }

    rsdp1_t* v1 = rsdp.v1;

    const char expected[8] = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ' };
    if (memcmp(v1->signature, expected, 8)) {
        printk("[ACPI] Invalid RSDP signature\n");
        return;
    }

    printk("[ACPI] OEM: ");
    for (size_t i = 0; i < 6; i++)
        printk("%c", v1->oem_id[i]);
    printk(", revision=%d\n", v1->revision);

    switch (rsdp.version) {
    case RSDP_VERSION_2: {
        rsdp2_t* v2 = rsdp.v2;

        if (!acpi_validate((char*)v2, v2->length)) {
            printk("[ACPI] RSDP v2 extended checksum failed\n");
            return;
        }

        printk("[ACPI] XSDT at physical %p\n", v2->xsdt_address);

        xsdt_t* xsdt = (xsdt_t*)P2V(v2->xsdt_address);

        if (!acpi_validate((char*)xsdt, xsdt->header.length)) {
            printk("[ACPI] XSDT checksum failed\n");
            return;
        }

        size_t num_items = (xsdt->header.length - sizeof(acpi_sdt_header_t)) / 8;
        printk("[ACPI] XSDT: %d entries\n", num_items);

        // Not really that useful
        for (size_t i = 0; i < num_items; i++) {
            acpi_sdt_header_t* h = (acpi_sdt_header_t*)P2V(xsdt->entries[i]);
            printk("[ACPI]   [%d] ", i);

            for (size_t j = 0; j < 4; j++)
                printk("%c ", h->signature[j]);

            printk("\n");
        }

        madt_t* madt = (madt_t*)acpi_find_table_xsdt(xsdt, num_items, "APIC");
        if (!madt) {
            printk("[ACPI] MADT not found in XSDT\n");
            return;
        }

        parse_madt(madt);
        break;
    }
    case RSDP_VERSION_1: {
        printk("[ACPI] RSDT at physical %p\n", v1->rsdt_address);

        rsdt_t* rsdt = (rsdt_t*)P2V(v1->rsdt_address);

        if (!acpi_validate((char*)rsdt, rsdt->header.length)) {
            printk("[ACPI] RSDT checksum failed\n");
            return;
        }

        size_t num_items = (rsdt->header.length - sizeof(acpi_sdt_header_t)) / 4;
        printk("[ACPI] RSDT: %d entries\n", num_items);

        // Not really that useful
        for (size_t i = 0; i < num_items; i++) {
            acpi_sdt_header_t* h = (acpi_sdt_header_t*)P2V(rsdt->entries[i]);
            printk("[ACPI]   [%d] ", i);

            for (size_t j = 0; j < 4; j++)
                printk("%c ", h->signature[j]);

            printk("\n");
        }

        madt_t* madt = (madt_t*)acpi_find_table_rsdt(rsdt, num_items, "APIC");
        if (!madt) {
            printk("[ACPI] MADT not found in RSDT\n");
            return;
        }

        parse_madt(madt);
        break;
    }
    default:
        printk("[ACPI] Unknown RSDP version\n");
        break;
    }
}

void pic_disable()
{
    outb(PIC_COMMAND_MASTER, ICW_1);
    outb(PIC_COMMAND_SLAVE, ICW_1);
    outb(PIC_DATA_MASTER, ICW_2_M);
    outb(PIC_DATA_SLAVE, ICW_2_S);
    outb(PIC_DATA_MASTER, ICW_3_M);
    outb(PIC_DATA_SLAVE, ICW_3_S);
    outb(PIC_DATA_MASTER, ICW_4);
    outb(PIC_DATA_SLAVE, ICW_4);
    outb(PIC_DATA_MASTER, 0xFF);
    outb(PIC_DATA_SLAVE, 0xFF);
}
