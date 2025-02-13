#ifndef PLATFORM_ABSTRACTION_VIRTUAL_H
#define PLATFORM_ABSTRACTION_VIRTUAL_H

#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, PageSize pageType,
                               U64 additionalFlags);
void mapVirtualRegion(U64 virt, PagedMemory memory, PageSize pageType);

#endif
