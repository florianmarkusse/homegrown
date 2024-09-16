#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

#include "interoperation/kernel-parameters.h"
#include "interoperation/types.h"
#include "memory/management/definitions.h"

void appendVirtualMemoryManagerStatus();

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

#endif
