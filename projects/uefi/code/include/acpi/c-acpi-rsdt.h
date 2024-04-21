#ifndef ACPI_C_ACPI_RSDT_H
#define ACPI_C_ACPI_RSDT_H

#include "efi/c-efi-base.h"

typedef struct __attribute__((packed)) {
    char signature[4];
    CEfiU32 length;
    CEfiU8 rev;
    CEfiU8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    CEfiU32 oem_rev;
    char creator_id[4];
    CEfiU32 creator_rev;
} CAcpiSDT;

// V1
#define RSDT_BYTE_LEN 4
// V2
#define XSDT_BYTE_LEN 8

typedef struct __attribute((packed)) {
    CAcpiSDT header;
    char ptrs_start[];
} CAcpiRSDT;

#endif
