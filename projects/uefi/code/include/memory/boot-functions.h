#ifndef MEMORY_BOOT_FUNCTIONS_H
#define MEMORY_BOOT_FUNCTIONS_H

#include "efi/c-efi-base.h"
#include "efi/c-efi-system.h"

CEfiPhysicalAddress allocAndZero(USize numPages);
void mapMemoryAt(U64 phys, U64 virt, U64 size);

typedef struct {
    USize memoryMapSize;
    CEfiMemoryDescriptor *memoryMap;
    USize mapKey;
    USize descriptorSize;
    U32 descriptorVersion;
} MemoryInfo;
MemoryInfo getMemoryInfo();

bool needsTobeMappedByOS(CEfiMemoryType type);

#endif
