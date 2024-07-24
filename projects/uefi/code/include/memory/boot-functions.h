#ifndef MEMORY_BOOT_FUNCTIONS_H
#define MEMORY_BOOT_FUNCTIONS_H

#include "efi/c-efi-base.h"   // for PhysicalAddress
#include "efi/c-efi-system.h" // for MemoryDescriptor, MemoryType
#include "types.h"            // for USize, U64, U32

PhysicalAddress allocAndZero(USize numPages);
void mapMemoryAt(U64 phys, U64 virt, U64 size);

typedef struct {
    USize memoryMapSize;
    MemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;
MemoryInfo getMemoryInfo();

bool needsTobeMappedByOS(MemoryType type);

#endif
