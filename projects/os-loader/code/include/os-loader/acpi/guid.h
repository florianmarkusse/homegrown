#ifndef EFI_ACPI_GUID_H
#define EFI_ACPI_GUID_H

#include "shared/types/types.h"
#include "shared/uuid.h"

static constexpr auto EFI_ACPI_20_TABLE_GUID =
    (UUID){.ms1 = 0x8868e871,
           .ms2 = 0xe4f1,
           .ms3 = 0x11d3,
           .ms4 = {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
static constexpr auto ACPI_TABLE_GUID =
    (UUID){.ms1 = 0xeb9d2d30,
           .ms2 = 0x2d88,
           .ms3 = 0x11d3,
           .ms4 = {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};

#endif
