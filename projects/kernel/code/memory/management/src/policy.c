#include "memory/management/policy.h"
#include "memory/management/physical.h"
#include "memory/management/virtual.h"

void *allocAndMap(PagedMemory_a request, PageType pageType) {
    U64 test = JUMBO_PAGE_SIZE;
    U64 size = pageType * request.len;
    U64 virtualAddress = getVirtualMemory(size, pageType);
    PagedMemory_a physicalAddresses = allocPhysicalPages(request, pageType);

    for (U64 i = 0; i < physicalAddresses.len; i++) {
        mapVirtualRegion(virtualAddress, physicalAddresses.buf[i], pageType);
    }

    return (void *)virtualAddress;
}

void *allocContiguousAndMap(U64 numberOfPages, PageType pageType) {
    U64 size = numberOfPages * pageType;

    U64 virtualAddress = getVirtualMemory(size, pageType);
    U64 physicalAddress = allocContiguousPhysicalPages(numberOfPages, pageType);

    mapVirtualRegion(virtualAddress,
                     (PagedMemory){.numberOfPages = numberOfPages,
                                   .pageStart = physicalAddress},
                     pageType);

    return (void *)virtualAddress;
}
