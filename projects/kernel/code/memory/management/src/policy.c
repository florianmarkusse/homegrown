#include "memory/management/policy.h"
#include "interoperation/memory/definitions.h"
#include "platform-abstraction/idt.h"
#include "platform-abstraction/memory/management/physical.h"
#include "platform-abstraction/memory/management/virtual.h"
#include "shared/maths/maths.h"

void *allocAndMap(U64 bytes) {
    PagedMemory pagedMemory[PageTableFormat.ENTRIES];

    PageSize pageSize;
    U64 inPages;
    PagedMemory_a request;
    // Note that we do the final possible pageSize outside the loop.
    for (U64 i = 0; i < NUM_PAGE_SIZES - 1; i++) {
        pageSize = pageSizes[i];
        inPages = CEILING_DIV_VALUE(bytes, pageSize);

        if (i < NUM_PAGE_SIZES - 1 && inPages <= PageTableFormat.ENTRIES / 2) {
            request = (PagedMemory_a){.buf = pagedMemory, .len = inPages};
            return allocAndMapExplicit(request, pageSize);
        }
    }

    pageSize = pageSizes[NUM_PAGE_SIZES - 1];
    inPages = CEILING_DIV_VALUE(bytes, pageSize);

    if (inPages > PageTableFormat.ENTRIES) {
        triggerFault(FAULT_TOO_LARGE_ALLOCATION);
    }

    request = (PagedMemory_a){.buf = pagedMemory, .len = inPages};
    return allocAndMapExplicit(request, pageSize);
}

void *allocAndMapExplicit(PagedMemory_a request, PageSize pageSize) {
    U64 size = pageSize * request.len;
    U64 virtualAddress = getVirtualMemory(size, pageSize);
    PagedMemory_a physicalAddresses = allocPhysicalPages(request, pageSize);

    U64 virtualRegion = virtualAddress;
    for (U64 i = 0; i < physicalAddresses.len; i++) {
        mapVirtualRegion(virtualRegion, physicalAddresses.buf[i], pageSize);
        virtualRegion += physicalAddresses.buf[i].numberOfPages * pageSize;
    }

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    return (void *)virtualAddress;
}

void *allocContiguousAndMap(U64 numberOfPages, PageSize pageSize) {
    U64 size = numberOfPages * pageSize;

    U64 virtualAddress = getVirtualMemory(size, pageSize);
    U64 physicalAddress = allocContiguousPhysicalPages(numberOfPages, pageSize);

    mapVirtualRegion(virtualAddress,
                     (PagedMemory){.numberOfPages = numberOfPages,
                                   .pageStart = physicalAddress},
                     pageSize);

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
