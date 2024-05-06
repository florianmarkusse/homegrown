#ifndef ACPI_C_ACPI_RSDT_H
#define ACPI_C_ACPI_RSDT_H

#include "acpi/c-acpi-rdsp.h"
#include "efi/c-efi-base.h"

#define ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN 4

typedef struct __attribute((packed)) {
    unsigned char signature[ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN];
    CEfiU32 length;
    CEfiU8 rev;
    CEfiU8 checksum;
    unsigned char oem_id[6];
    unsigned char oem_table_id[8];
    CEfiU32 oem_rev;
    unsigned char creator_id[4];
    CEfiU32 creator_rev;
} CAcpiDescriptionTableHeader;

typedef struct __attribute((packed)) {
    CAcpiDescriptionTableHeader header;
    void **descriptionHeaders;
} CAcpiSDT;

void printDescriptionHeaders(RSDPResult rsdp);

#endif
