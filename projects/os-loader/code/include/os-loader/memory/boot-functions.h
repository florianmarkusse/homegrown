#ifndef OS_LOADER_MEMORY_BOOT_FUNCTIONS_H
#define OS_LOADER_MEMORY_BOOT_FUNCTIONS_H

#include "efi/firmware/base.h" // for PhysicalAddress
#include "efi-to-kernel/memory/descriptor.h"
#include "shared/types/types.h" // for USize, U64, U32

PhysicalAddress allocAndZero(USize numPages);
void mapMemoryAt(U64 phys, U64 virt, U64 size);
void mapMemoryAtWithFlags(U64 phys, U64 virt, U64 size, U64 additionalFlags);

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
