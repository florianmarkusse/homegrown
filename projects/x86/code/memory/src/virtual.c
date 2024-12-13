#include "platform-abstraction/memory/management/virtual.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/memory/descriptor.h"
#include "platform-abstraction/cpu.h"
#include "platform-abstraction/memory/management/physical.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/macros.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/pat.h"
#include "x86/memory/physical.h"

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
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
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

U64 getPhysicalAddressFrame(U64 virtualPage) {
    return virtualPage & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
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

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
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

        U64 pageSize = JUMBO_PAGE_SIZE;
        for (U8 i = 0; i < depth; i++, pageSize /= PageTableFormat.ENTRIES) {
            U64 *address = &(currentTable->pages[RING_RANGE_VALUE(
                (virtual / pageSize), PageTableFormat.ENTRIES)]);

            if (i == depth - 1) {
                U64 value = VirtualPageMasks.PAGE_PRESENT |
                            VirtualPageMasks.PAGE_WRITABLE | physical |
                            additionalFlags;
                if (pageType == HUGE_PAGE || pageType == LARGE_PAGE) {
                    value |= VirtualPageMasks.PAGE_EXTENDED_SIZE;
                }
                *address = value;
            } else if (!*address) {
                U64 value = VirtualPageMasks.PAGE_PRESENT |
                            VirtualPageMasks.PAGE_WRITABLE;
                value |= getZeroBasePage();
                *address = value;
            }

            currentTable =
                /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                (VirtualPageTable *)ALIGN_DOWN_EXP(*address, PAGE_FRAME_SHIFT);
        }
    }
}

static bool isExtendedPageLevel(U8 level) { return level == 1 || level == 2; }

MappedPage getMappedPage(U64 virtual) {
    U64 pageSize = JUMBO_PAGE_SIZE;
    VirtualPageTable *currentTable = level4PageTable;
    MappedPage result;
    result.pageSize = WUMBO_PAGE_SIZE;
    U64 *address;
    U8 totalDepth = pageSizeToDepth(BASE_PAGE);
    for (U8 level = 0; level < totalDepth;
         level++, pageSize /= PageTableFormat.ENTRIES) {
        address = &(currentTable->pages[RING_RANGE_VALUE(
            (virtual / pageSize), PageTableFormat.ENTRIES)]);
        result.pageSize /= PageTableFormat.ENTRIES;

        if (isExtendedPageLevel(level) &&
            ((*address) & VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
            result.entry = *(VirtualEntry *)address;
            return result;
        }

        currentTable =
            /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
            (VirtualPageTable *)ALIGN_DOWN_EXP(*address, PAGE_FRAME_SHIFT);
    }

    result.entry = *(VirtualEntry *)address;
    return result;
}
