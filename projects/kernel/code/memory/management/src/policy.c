#include "memory/management/policy.h"
#include "cpu/idt.h"
#include "interoperation/memory/definitions.h"
#include "log/log.h"
#include "memory/management/physical.h"
#include "memory/management/virtual.h"
#include "util/maths.h"

void *allocAndMap(U64 bytes) {
    PagedMemory pagedMemory[PAGE_TABLE_ENTRIES];

    PageSize pageSize;
    U64 inPages;
    PagedMemory_a request;
    // Note that we do the final possible pageSize outside the loop.
    for (U64 i = 0; i < NUM_PAGE_SIZES - 1; i++) {
        pageSize = pageSizes[i];
        inPages = CEILING_DIV_VALUE(bytes, pageSize);

        if (i < NUM_PAGE_SIZES - 1 && inPages <= PAGE_TABLE_ENTRIES / 2) {
            request = (PagedMemory_a){.buf = pagedMemory, .len = inPages};
            return allocAndMapExplicit(request, pageSize);
        }
    }

    pageSize = pageSizes[NUM_PAGE_SIZES - 1];
    inPages = CEILING_DIV_VALUE(bytes, pageSize);

    if (inPages > PAGE_TABLE_ENTRIES) {
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

    return (void *)virtualAddress;
}

void freeMapped(U64 start, U64 bytes) {
    ASSERT((start >> PAGE_FRAME_SHIFT));

    MappedPage page = getMappedPage(start);

    FLUSH_AFTER {
        LOG(STRING("Got virtual page:\t"));
        LOG(page.entry.value, NEWLINE);
        LOG(STRING("Page Size:\t"));
        LOG(page.pageSize, NEWLINE);
    }
}
