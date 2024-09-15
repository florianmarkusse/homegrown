#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

#include "interoperation/kernel-parameters.h"
#include "interoperation/types.h"
#include "memory/management/definitions.h"

void printVirtualMemoryManagerStatus();

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageType alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageType pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageType pageType);
#endif
