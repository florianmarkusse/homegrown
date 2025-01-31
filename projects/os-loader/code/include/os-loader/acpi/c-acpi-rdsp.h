#ifndef EFI_ACPI_C_ACPI_RDSP_H
#define EFI_ACPI_C_ACPI_RDSP_H

#include "os-loader/acpi/configuration-table.h"
#include "shared/types/types.h"

typedef struct __attribute((packed)) {
    I8 signature[8];
    U8 checksum;
    I8 oem_id[6];
    U8 rev;
    U32 rsdt_addr;
} CAcpiRSDPV1;

typedef struct __attribute__((packed)) {
    CAcpiRSDPV1 v1;
    U32 length;
    U64 xsdt_addr;
    U8 ext_checksum;
    U8 reserved[3];
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

RSDPResult getRSDP(USize tableEntries, ConfigurationTable *tables);

#endif
