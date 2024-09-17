#include "memory/management/policy.h"
#include "memory/management/physical.h"
#include "memory/management/virtual.h"

void *allocAndMap(PagedMemory_a request, PageSize pageSize) {
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
