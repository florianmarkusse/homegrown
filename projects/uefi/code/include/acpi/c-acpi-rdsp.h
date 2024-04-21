#ifndef ACPI_C_ACPI_RDSP_H
#define ACPI_C_ACPI_RDSP_H

#include "efi/c-efi-base.h"
typedef struct __attribute__((packed)) {
    char signature[8];
    CEfiU8 checksum;
    char oem_id[6];
    CEfiU8 rev;
    CEfiU32 rsdt_addr;
    // Revision 2 only after this comment
    CEfiU32 length;
    CEfiU64 xsdt_addr;
    CEfiU8 ext_checksum;
    CEfiU8 reserved[3];
} CAcpiRSDP;

#endif
