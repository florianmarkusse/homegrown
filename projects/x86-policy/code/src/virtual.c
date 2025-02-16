#include "x86-policy/virtual.h"

#include "efi-to-kernel/memory/definitions.h"
#include "efi-to-kernel/memory/descriptor.h"
#include "platform-abstraction/cpu.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
#include "x86-virtual.h"
#include "x86/configuration/cpu2.h"
#include "x86/memory/pat.h"

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

void initVirtualMemoryManager(KernelMemory kernelMemory) {
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
    level4PageTable = (VirtualPageTable *)CR3();
    programPat();
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
