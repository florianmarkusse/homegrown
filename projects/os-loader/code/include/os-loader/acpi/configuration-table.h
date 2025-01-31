#ifndef CONFIGURATION_TABLE_H
#define CONFIGURATION_TABLE_H

#include "shared/uuid.h"
typedef struct {
    UUID vendor_guid;
    void *vendor_table;
} ConfigurationTable;

#endif
