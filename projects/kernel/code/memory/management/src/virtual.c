#include "memory/management/virtual.h"

#include "cpu/x86.h"
#include "interoperation/assert.h"
#include "interoperation/macros.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/memory/descriptor.h"
#include "interoperation/types.h"
#include "memory/management/definitions.h"
#include "memory/management/physical.h"
#include "memory/manipulation/manipulation.h"
#include "shared/maths/maths.h"

VirtualPageTable *level4PageTable;

VirtualRegion higherHalfRegion = {.start = HIGHER_HALF_START,
                                  .end = KERNEL_SPACE_START};
// Start is set in the init function.
VirtualRegion lowerHalfRegion = {.start = 0, .end = LOWER_HALF_END};

static U8 pageSizeToDepth(PageSize pageSize) {
    switch (pageSize) {
    case BASE_PAGE: {
        return 4;
    }
    case LARGE_PAGE: {
        return 3;
    }
    default: {
        return 2;
    }
    }
}

static U64 getZeroBasePage() {
    PagedMemory memoryForAddresses[1];
    PagedMemory_a memory =
        allocPhysicalPages((PagedMemory_a){.buf = memoryForAddresses,
                                           .len = COUNTOF(memoryForAddresses)},
                           BASE_PAGE);
    memset((void *)memory.buf[0].pageStart, 0, PAGE_FRAME_SIZE);
    return memory.buf[0].pageStart;
}

U64 getVirtualMemory(U64 size, PageSize alignValue) {
    ASSERT(size <= JUMBO_PAGE_SIZE);
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, alignValue);

    ASSERT(higherHalfRegion.start <= alignedUpValue);
    ASSERT(alignedUpValue <= higherHalfRegion.end);
    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
           higherHalfRegion.start);

    higherHalfRegion.start = alignedUpValue;
    U64 result = higherHalfRegion.start;

    higherHalfRegion.start += size;
    return result;
}

static void programPat() {
    PAT patValues = {.value = rdmsr(PAT_LOCATION)};

    patValues.pats[3].pat = PAT_WRITE_COMBINGING_WC;
    wrmsr(PAT_LOCATION, patValues.value);

    flushTLB();
}

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory) {
    U64 currentHighestAddress = 0;
    for (U64 i = 0;
         i < kernelMemory.totalDescriptorSize / kernelMemory.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors +
                                 (i * kernelMemory.descriptorSize));
        U64 highestAddress =
            desc->virtualStart + desc->numberOfPages * PAGE_FRAME_SIZE;

        if (highestAddress > currentHighestAddress) {
            currentHighestAddress = highestAddress;
        }
    }
    lowerHalfRegion.start = currentHighestAddress;

    level4PageTable = (VirtualPageTable *)level4Address;
    programPat();
}

void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType) {
    mapVirtualRegionWithFlags(virtual, memory, pageType, 0);
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virtual) >> 48L) == 0 || ((virtual) >> 48L) == 0xFFFF);

    U8 depth = pageSizeToDepth(pageType);

    ASSERT(!(RING_RANGE_VALUE(virtual, pageType)));
    ASSERT(!(RING_RANGE_VALUE(memory.pageStart, pageType)));

    U64 virtualEnd = virtual + pageType * memory.numberOfPages;
    for (U64 physical = memory.pageStart; virtual < virtualEnd;
         virtual += pageType, physical += pageType) {
        VirtualPageTable *currentTable = level4PageTable;

        U64 indexShift = LEVEL_4_SHIFT;
        for (U8 i = 0; i < depth; i++, indexShift -= PAGE_TABLE_SHIFT) {
            U64 *address = &(currentTable->pages[RING_RANGE_EXP(
                (virtual >> indexShift), PAGE_TABLE_SHIFT)]);

            if (i == depth - 1) {
                U64 value =
                    PAGE_PRESENT | PAGE_WRITABLE | physical | additionalFlags;
                if (pageType == HUGE_PAGE || pageType == LARGE_PAGE) {
                    value |= PAGE_EXTENDED_SIZE;
                }
                *address = value;
            } else if (!*address) {
                U64 value = PAGE_PRESENT | PAGE_WRITABLE;
                value |= getZeroBasePage();
                *address = value;
            }

            currentTable =
                (VirtualPageTable *)ALIGN_DOWN_EXP(*address, PAGE_FRAME_SHIFT);
        }
    }
}

static bool isExtendedPageLevel(U8 level) { return level == 1 || level == 2; }

MappedPage getMappedPage(U64 virtual) {
    U64 indexShift = LEVEL_4_SHIFT;
    VirtualPageTable *currentTable = level4PageTable;
    MappedPage result;
    result.pageSize = WUMBO_PAGE_SIZE;
    U64 *address;
    U8 totalDepth = pageSizeToDepth(BASE_PAGE);
    for (U8 level = 0; level < totalDepth;
         level++, indexShift -= PAGE_TABLE_SHIFT) {
        address = &(currentTable->pages[RING_RANGE_EXP((virtual >> indexShift),
                                                       PAGE_TABLE_SHIFT)]);
        result.pageSize >>= PAGE_TABLE_SHIFT;

        if (isExtendedPageLevel(level) && ((*address) & PAGE_EXTENDED_SIZE)) {
            result.entry = *(VirtualEntry *)address;
            return result;
        }

        currentTable =
            (VirtualPageTable *)ALIGN_DOWN_EXP(*address, PAGE_FRAME_SHIFT);
    }

    result.entry = *(VirtualEntry *)address;
    return result;
}
