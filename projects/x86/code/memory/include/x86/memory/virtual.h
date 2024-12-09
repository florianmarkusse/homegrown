#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "interoperation/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

extern PageSize pageSizes[NUM_PAGE_SIZES];

static inline U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

extern VirtualPageTable *level4PageTable;

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

MappedPage getMappedPage(U64 virtual);

#define BYTES_TO_PAGE_FRAMES(a)                                                \
    (((a) >> PAGE_FRAME_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

#endif
