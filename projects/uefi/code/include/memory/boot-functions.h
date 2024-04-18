#ifndef MEMORY_BOOT_FUNCTIONS_H
#define MEMORY_BOOT_FUNCTIONS_H

#include "efi/c-efi-base.h"
#include "efi/c-efi-system.h"

CEfiPhysicalAddress allocAndZero(CEfiUSize numPages);
void mapMemory(CEfiU64 phys, CEfiU64 virt, CEfiU32 size);

typedef struct {
    CEfiUSize memoryMapSize;
    CEfiMemoryDescriptor *memoryMap;
    CEfiUSize mapKey;
    CEfiUSize descriptorSize;
    CEfiU32 descriptorVersion;
} MemoryInfo;
MemoryInfo getMemoryInfo();

bool needsTobeMappedByOS(CEfiMemoryType type);

#endif
