#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi-to-kernel/memory/descriptor.h"
#include "efi/firmware/base.h"  // for PhysicalAddress
#include "shared/types/types.h" // for USize, U64, U32

PhysicalAddress allocAndZero(USize numPages);

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;
MemoryInfo getMemoryInfo();

#endif
