#ifndef EFI_PROTOCOL_ACPI_H
#define EFI_PROTOCOL_ACPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define ACPI_TABLE_PROTOCOL_GUID                                         \
    EFI_GUID(0xffe06bdd, 0x6107, 0x46a6, 0x7b, 0xb2, 0x5a, 0x9c, 0x7e, 0xc5, \
               0x27, 0x5c)

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
