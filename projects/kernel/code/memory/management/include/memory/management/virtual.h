#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

// TODO: Move this to x86

#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "shared/types/types.h"
#include "memory/management/definitions.h"
#include "x86/memory/virtual.h"

static inline U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
}

typedef struct {
    U64 start;
    U64 end;
} VirtualRegion;

typedef struct {
    U64 pages[PageTableFormat.ENTRIES];
} VirtualPageTable;

extern VirtualPageTable *level4PageTable;

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

typedef struct {
    VirtualEntry entry;
    PageSize pageSize;
} MappedPage;
MappedPage getMappedPage(U64 virtual);

#endif
