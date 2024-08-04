#ifndef CONFIGURATION_TABLE_H
#define CONFIGURATION_TABLE_H

#include "acpi/guid.h"
typedef struct {
    Guid vendor_guid;
    void *vendor_table;
} ConfigurationTable;

#endif
