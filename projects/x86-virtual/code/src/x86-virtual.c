#include "x86-virtual.h"

#include "platform-abstraction/memory/manipulation.h"
#include "platform-abstraction/physical/allocation.h"
#include "platform-abstraction/virtual/map.h"
#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

VirtualPageTable *level4PageTable;

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
    U64 address = allocate4KiBPage(1);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    memset((void *)address, 0, PAGE_FRAME_SIZE);
    return address;
}

void mapVirtualRegion(U64 virt, PagedMemory memory, PageSize pageType) {
    mapVirtualRegionWithFlags(virt, memory, pageType, 0);
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegionWithFlags(U64 virt, PagedMemory memory, PageSize pageType,
                               U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0 || ((virt) >> 48L) == 0xFFFF);

    U8 depth = pageSizeToDepth(pageType);

    ASSERT(!(RING_RANGE_VALUE(virt, pageType)));
    ASSERT(!(RING_RANGE_VALUE(memory.pageStart, pageType)));

    U64 virtualEnd = virt + pageType * memory.numberOfPages;
    for (U64 physical = memory.pageStart; virt < virtualEnd;
         virt += pageType, physical += pageType) {
        VirtualPageTable *currentTable = level4PageTable;

        U64 pageSize = JUMBO_PAGE_SIZE;
        for (U8 i = 0; i < depth; i++, pageSize /= PageTableFormat.ENTRIES) {
            U64 *address = &(currentTable->pages[RING_RANGE_VALUE(
                (virt / pageSize), PageTableFormat.ENTRIES)]);

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
