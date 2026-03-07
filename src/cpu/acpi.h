#ifndef CPU_ACPI_H
#define CPU_ACPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Root System Description Pointer v2
typedef struct {
    char signature[8]; // = "RSD PTR ", no '\0'
    uint8_t checksum;  // sum of first 20 bytes mod 256 == 0
    char oem_id[6];
    uint8_t revision; // 0 = ACPI 1.0, 2 = ACPI 2.0
    uint32_t rsdt_address;
} __attribute__((packed)) rsdp1_t;

// Root System Description Pointer v2
typedef struct {
    rsdp1_t v1;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp2_t;

// ACPI System Description Table Header
typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

// Root System Descriptor Table (ACPI v1)
typedef struct {
    acpi_sdt_header_t header;
    uint32_t entries[];
} __attribute__((packed)) rsdt_t;

// Extended System Descriptor Table (ACPI v2)
typedef struct {
    acpi_sdt_header_t header;
    uint64_t entries[];
} __attribute__((packed)) xsdt_t;

typedef enum {
    RSDP_VERSION_NONE = 0,
    RSDP_VERSION_1 = 1,
    RSDP_VERSION_2 = 2,
} rsdp_version_t;

typedef struct {
    rsdp_version_t version;
    union {
        rsdp1_t* v1;
        rsdp2_t* v2;
    };
} rsdp_t;

// Multiple APIC description Table
typedef struct {
    acpi_sdt_header_t header;
    uint32_t lapic_address; // physical address of local APIC
    uint32_t flags;         // bit 0: dual 8259 PICs installed
} __attribute__((packed)) madt_t;

// Common header for all MADT interrupt controller entries
typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_entry_header_t;

// Processor Local APIC
typedef struct {
    madt_entry_header_t header; // type = 0
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags; // bit 0: enabled, bit 1: online capable
} __attribute__((packed)) madt_entry_lapic_t;

// I/O APIC
typedef struct {
    madt_entry_header_t header; // type = 1
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address; // physical address of I/O APIC registers
    uint32_t gsi_base;       // global system interrupt base
} __attribute__((packed)) madt_entry_ioapic_t;

// Interrupt Source Override
typedef struct {
    madt_entry_header_t header; // type = 2
    uint8_t bus;                // always 0 (ISA)
    uint8_t source;             // ISA IRQ source
    uint32_t gsi;               // global system interrupt target
    uint16_t flags;             // polarity and trigger mode
} __attribute__((packed)) madt_entry_iso_t;

void acpi_init(rsdp_t rsdp);
void pic_disable();

#endif // CPU_ACPI_H
