#ifndef ABSTRACTION_MEMORY_VIRTUAL_MAP_H
#define ABSTRACTION_MEMORY_VIRTUAL_MAP_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, U64 pageSize,
                               U64 additionalFlags);
void mapVirtualRegion(U64 virt, PagedMemory memory, U64 pageSize);

#endif
