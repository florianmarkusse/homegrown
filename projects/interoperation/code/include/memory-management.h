#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

// TODO: This is a temporary home for the memory types. I prefer to strip off
// all the UEFI stuff in the uefi part after which only the necessary data is
// passed to the kernel as kernel parameters.

#include "types.h"
typedef enum MemoryType {
    RESERVED_MEMORY_TYPE,
    LOADER_CODE,
    LOADER_DATA,
    BOOT_SERVICES_CODE,
    BOOT_SERVICES_DATA,
    RUNTIME_SERVICES_CODE,
    RUNTIME_SERVICES_DATA,
    CONVENTIONAL_MEMORY,
    UNUSABLE_MEMORY,
    ACPI_RECLAIM_MEMORY,
    ACPI_MEMORY_NVS,
    MEMORY_MAPPED_IO,
    MEMORY_MAPPED_IO_PORT_SPACE,
    PAL_CODE,
    PERSISTENT_MEMORY,
    MEMORY_TYPE_N,
} MemoryType;

#define MEMORY_UC U64_C(0x0000000000000001)
#define MEMORY_WC U64_C(0x0000000000000002)
#define MEMORY_WT U64_C(0x0000000000000004)
#define MEMORY_WB U64_C(0x0000000000000008)
#define MEMORY_UCE U64_C(0x0000000000000010)
#define MEMORY_WP U64_C(0x0000000000001000)
#define MEMORY_RP U64_C(0x0000000000002000)
#define MEMORY_XP U64_C(0x0000000000004000)
#define MEMORY_NV U64_C(0x0000000000008000)
#define MEMORY_MORE_RELIABLE U64_C(0x0000000000010000)
#define MEMORY_RO U64_C(0x0000000000020000)
#define MEMORY_RUNTIME U64_C(0x8000000000000000)

#define MEMORY_DESCRIPTOR_VERSION U32_C(0x00000001)

typedef struct MemoryDescriptor {
    U32 type;
    U64 physical_start;
    U64 virtual_start;
    U64 number_of_pages;
    U64 attribute;
} MemoryDescriptor;

#endif
