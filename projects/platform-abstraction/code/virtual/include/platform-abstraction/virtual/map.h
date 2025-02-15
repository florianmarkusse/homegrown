#ifndef PLATFORM_ABSTRACTION_MAP_H
#define PLATFORM_ABSTRACTION_MAP_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, PageSize pageType,
                               U64 additionalFlags);
void mapVirtualRegion(U64 virt, PagedMemory memory, PageSize pageType);

#endif
