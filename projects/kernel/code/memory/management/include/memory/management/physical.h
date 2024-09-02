#ifndef MEMORY_MANAGEMENT_PHYSICAL_H
#define MEMORY_MANAGEMENT_PHYSICAL_H

#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "memory/management/definitions.h"

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

typedef struct {
    union {
        U8 data[LARGE_PAGE_SIZE];
        PhysicalBasePage basePages[PAGE_TABLE_ENTRIES];
    };
} PhysicalLargePage;

typedef struct {
    union {
        U8 data[HUGE_PAGE_SIZE];
        PhysicalBasePage basePages[PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES];
        PhysicalLargePage largePages[PAGE_TABLE_ENTRIES];
    };
} PhysicalHugePage;

void initPhysicalMemoryManager(KernelMemory kernelMemory);

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageType pageType);
PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageType pageType);

void freePhysicalPage(PagedMemory page, PageType pageType);
void freePhysicalPages(PagedMemory_a pages, PageType pageType);

void printPhysicalMemoryManagerStatus();

#define GET_PHYSICAL(pages, t) (typeof(t))allocPhysicalPages(pages)

#endif
