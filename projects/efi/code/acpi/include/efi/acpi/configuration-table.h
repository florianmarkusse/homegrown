#ifndef EFI_ACPI_CONFIGURATION_TABLE_H
#define EFI_ACPI_CONFIGURATION_TABLE_H

#include "shared/uuid.h"
typedef struct {
    UUID vendor_guid;
    void *vendor_table;
} ConfigurationTable;

#endif
