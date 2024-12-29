#ifndef EFI_ACPI_CONFIGURATION_TABLE_H
#define EFI_ACPI_CONFIGURATION_TABLE_H

#include "uefi/guid.h"
typedef struct {
    Guid vendor_guid;
    void *vendor_table;
} ConfigurationTable;

#endif
