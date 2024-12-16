#include "platform-abstraction/memory/management/policy.h"

#include "platform-abstraction/idt.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"
#include "x86/memory/physical.h"
#include "x86/memory/virtual.h"
#include "shared/assert.h"
#include "x86/cpu/fault.h"

static constexpr U64 USED_PAGE_SIZES_MASK =
    (PAGE_FRAME_SIZE | LARGE_PAGE_SIZE | HUGE_PAGE_SIZE);
static bool isPageSizeInUse(U64 pageSize) {
    ASSERT(((pageSize) & (pageSize - 1)) == 0);
    return pageSize & USED_PAGE_SIZES_MASK;
}

typedef struct {
    U64 numberOfPages;
    PageSize pageSize;
} PageSizeConversion;

static PageSizeConversion convertBytesToPages(U64 bytesPowerOfTwo) {
    ASSERT(((bytesPowerOfTwo) & (bytesPowerOfTwo - 1)) == 0);
    if (bytesPowerOfTwo <= PAGE_FRAME_SIZE) {
        return (PageSizeConversion){.numberOfPages = 1,
                                    .pageSize = PAGE_FRAME_SIZE};
    }
    PageSizeConversion result =
        (PageSizeConversion){.numberOfPages = 1, .pageSize = bytesPowerOfTwo};
    while (!isPageSizeInUse(result.pageSize)) {
        result.numberOfPages <<= 1;
        result.pageSize >>= 1;
    }
    return result;
}

void *allocAndMapExplicit(U64 numberOfPages, U64 preferredPageSizePowerOfTwo) {
    PageSizeConversion conversion =
        convertBytesToPages(preferredPageSizePowerOfTwo);

    numberOfPages *= conversion.numberOfPages;
    if (numberOfPages > PageTableFormat.ENTRIES) {
        triggerFault(FAULT_TOO_LARGE_ALLOCATION);
    }
    PagedMemory pagedMemory[PageTableFormat.ENTRIES];
    PagedMemory_a request =
        (PagedMemory_a){.buf = pagedMemory, .len = numberOfPages};

    U64 size = conversion.pageSize * request.len;
    U64 virtualAddress = getVirtualMemory(size, conversion.pageSize);
    PagedMemory_a physicalAddresses =
        allocPhysicalPages(request, conversion.pageSize);

    U64 virtualRegion = virtualAddress;
    for (U64 i = 0; i < physicalAddresses.len; i++) {
        mapVirtualRegion(virtualRegion, physicalAddresses.buf[i],
                         conversion.pageSize);
        virtualRegion +=
            physicalAddresses.buf[i].numberOfPages * conversion.pageSize;
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    return (void *)virtualAddress;
}

void *allocAndMap(U64 bytes) {
    PageSize pageSize;
    U64 inPages;
    // Note that we do the final possible pageSize outside the loop.
    for (U64 i = 0; i < MEMORY_PAGE_SIZES_COUNT - 1; i++) {
        pageSize = pageSizes[i];
        inPages = CEILING_DIV_VALUE(bytes, pageSize);

        if (i < MEMORY_PAGE_SIZES_COUNT - 1 &&
            inPages <= PageTableFormat.ENTRIES / 2) {
            return allocAndMapExplicit(inPages, pageSize);
        }
    }

    pageSize = pageSizes[MEMORY_PAGE_SIZES_COUNT - 1];
    inPages = CEILING_DIV_VALUE(bytes, pageSize);

    return allocAndMapExplicit(inPages, pageSize);
}

void *allocContiguousAndMap(U64 numberOfPages,
                            U64 preferredPageSizePowerOfTwo) {
    PageSizeConversion conversion =
        convertBytesToPages(preferredPageSizePowerOfTwo);
    numberOfPages *= conversion.numberOfPages;
    if (numberOfPages > PageTableFormat.ENTRIES) {
        triggerFault(FAULT_TOO_LARGE_ALLOCATION);
    }

    U64 size = numberOfPages * conversion.pageSize;

    U64 virtualAddress = getVirtualMemory(size, conversion.pageSize);
    U64 physicalAddress =
        allocContiguousPhysicalPages(numberOfPages, conversion.pageSize);

    mapVirtualRegion(virtualAddress,
                     (PagedMemory){.numberOfPages = numberOfPages,
                                   .pageStart = physicalAddress},
                     conversion.pageSize);

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    return (void *)virtualAddress;
}

void freeMapped(U64 start, U64 bytes) {
    U64 end = start + bytes;

    MappedPage page = getMappedPage(start);
    if (RING_RANGE_VALUE(start, page.pageSize)) {
        start = ALIGN_UP_VALUE(start, page.pageSize);
        if (start >= end) {
            return;
        }
        page = getMappedPage(start);
    }

    U64 physicalOfPage = getPhysicalAddressFrame(page.entry.value);

    PagedMemory pagedEntry =
        (PagedMemory){.pageStart = physicalOfPage, .numberOfPages = 1};

    PageSize previousPageSize = page.pageSize;
    U64 nextPhysical = physicalOfPage + previousPageSize;

    start += page.pageSize;
    while (start < end) {
        page = getMappedPage(start);
        physicalOfPage = getPhysicalAddressFrame(page.entry.value);

        if (physicalOfPage == nextPhysical &&
            previousPageSize == page.pageSize) {
            pagedEntry.numberOfPages++;
            nextPhysical += previousPageSize;
        } else {
            freePhysicalPage(pagedEntry, previousPageSize);

            pagedEntry =
                (PagedMemory){.pageStart = physicalOfPage, .numberOfPages = 1};

            previousPageSize = page.pageSize;
            nextPhysical = physicalOfPage + previousPageSize;
        }

        start += page.pageSize;
    }

    freePhysicalPage(pagedEntry, previousPageSize);
}
