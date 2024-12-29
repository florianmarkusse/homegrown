#ifndef EFI_TO_KERNEL_MEMORY_DESCRIPTOR_H
#define EFI_TO_KERNEL_MEMORY_DESCRIPTOR_H

// TODO: This is a temporary home for the memory types. I prefer to strip off
// all the UEFI stuff in the uefi part after which only the necessary data is
// passed to the kernel as kernel parameters.

#include "shared/types/types.h"
typedef enum MemoryType : U32 {
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

static constexpr U64 MEMORY_UC = 0x0000000000000001;
static constexpr U64 MEMORY_WC = 0x0000000000000002;
static constexpr U64 MEMORY_WT = 0x0000000000000004;
static constexpr U64 MEMORY_WB = 0x0000000000000008;
static constexpr U64 MEMORY_UCE = 0x0000000000000010;
static constexpr U64 MEMORY_WP = 0x0000000000001000;
static constexpr U64 MEMORY_RP = 0x0000000000002000;
static constexpr U64 MEMORY_XP = 0x0000000000004000;
static constexpr U64 MEMORY_NV = 0x0000000000008000;
static constexpr U64 MEMORY_MORE_RELIABLE = 0x0000000000010000;
static constexpr U64 MEMORY_RO = 0x0000000000020000;
static constexpr U64 MEMORY_RUNTIME = 0x8000000000000000;

static constexpr U32 MEMORY_DESCRIPTOR_VERSION = 0x00000001;

typedef struct MemoryDescriptor {
    MemoryType type;
    U64 physicalStart;
    U64 virtualStart;
    U64 numberOfPages;
    U64 attribute;
} MemoryDescriptor;

#endif
