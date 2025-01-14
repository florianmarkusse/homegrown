#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

extern VirtualPageTable *level4PageTable;

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

#define BYTES_TO_PAGE_FRAMES(a)                                                \
    (((a) >> PAGE_FRAME_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getPhysicalAddressFrame(U64 virtualPage);
U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

MappedPage getMappedPage(U64 virtual);

#endif
