#ifndef ACPI_C_ACPI_RDSP_H
#define ACPI_C_ACPI_RDSP_H

#include "efi/c-efi-base.h"

typedef struct __attribute((packed)) {
    char signature[8];
    CEfiU8 checksum;
    char oem_id[6];
    CEfiU8 rev;
    CEfiU32 rsdt_addr;
} CAcpiRSDPV1;

typedef struct __attribute__((packed)) {
    CAcpiRSDPV1 v1;
    CEfiU32 length;
    CEfiU64 xsdt_addr;
    CEfiU8 ext_checksum;
    CEfiU8 reserved[3];
} CAcpiRSDPV2;

typedef enum {
    RSDP_REVISION_1,
    RSDP_REVISION_2,
} RSDPRevision;

typedef union {
    CAcpiRSDPV1 v1;
    CAcpiRSDPV2 v2;
} RSDPData;

typedef struct {
    RSDPData *rsdp;
    RSDPRevision revision;
} RSDPResult;

RSDPResult getRSDP();

#endif
