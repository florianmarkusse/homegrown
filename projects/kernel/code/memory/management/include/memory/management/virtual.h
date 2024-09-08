#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

#include "interoperation/types.h"
#include "memory/management/definitions.h"

void printVirtualMemoryManagerStatus();

void initVirtualMemoryManager(U64 level4Address);

void mapVirtualRegion(U64 virtual, PagedMemory memory, PageType pageType,
                      U64 additionalFlags);
#endif
