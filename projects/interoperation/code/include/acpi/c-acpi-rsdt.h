#ifndef ACPI_C_ACPI_RSDT_H
#define ACPI_C_ACPI_RSDT_H

#include "types.h"

#define ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN 4

typedef struct __attribute((packed)) {
    unsigned char signature[ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN];
    U32 length;
    U8 rev;
    U8 checksum;
    unsigned char oem_id[6];
    unsigned char oem_table_id[8];
    U32 oem_rev;
    unsigned char creator_id[4];
    U32 creator_rev;
} CAcpiDescriptionTableHeader;

typedef struct __attribute((packed)) {
    CAcpiDescriptionTableHeader header;
    void **descriptionHeaders;
} CAcpiSDT;

#endif
