#ifndef EFI_C_EFI_PROTOCOL_ACPI_H
#define EFI_C_EFI_PROTOCOL_ACPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_ACPI_TABLE_PROTOCOL_GUID                                         \
    C_EFI_GUID(0xffe06bdd, 0x6107, 0x46a6, 0x7b, 0xb2, 0x5a, 0x9c, 0x7e, 0xc5, \
               0x27, 0x5c)

typedef struct CEfiACPITableProtocol {
    CEfiStatus(CEFICALL *installACPITable)(CEfiACPITableProtocol *this_,
                                           void *ACPITableBuffer,
                                           CEfiUSize ACPITableBufferSize,
                                           CEfiUSize *tableKey);
    CEfiStatus(CEFICALL *uninstallACPITable)(CEfiACPITableProtocol *this_,
                                             CEfiUSize tableKey);
} CEfiACPITableProtocol;

#ifdef __cplusplus
}
#endif

#endif
