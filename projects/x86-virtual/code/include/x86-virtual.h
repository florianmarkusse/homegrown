#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

extern VirtualPageTable *level4PageTable;

void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, PageSize pageType,
                               U64 additionalFlags);
void mapVirtualRegion(U64 virt, PagedMemory memory, PageSize pageType);

#endif
