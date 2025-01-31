#ifndef EFI_C_EFI_PROTOCOL_ACPI_H
#define EFI_C_EFI_PROTOCOL_ACPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os-loader/acpi/guid.h"
#include "os-loader/efi/c-efi-base.h"

static constexpr auto ACPI_TABLE_PROTOCOL_GUID =
    (GUID){.ms1 = 0xffe06bdd,
           .ms2 = 0x6107,
           .ms3 = 0x46a6,
           .ms4 = {0x7b, 0xb2, 0x5a, 0x9c, 0x7e, 0xc5, 0x27, 0x5c}};

typedef struct ACPITableProtocol {
    Status(EFICALL *installACPITable)(ACPITableProtocol *this_,
                                      void *ACPITableBuffer,
                                      USize ACPITableBufferSize,
                                      USize *tableKey);
    Status(EFICALL *uninstallACPITable)(ACPITableProtocol *this_,
                                        USize tableKey);
} ACPITableProtocol;

#ifdef __cplusplus
}
#endif

#endif
