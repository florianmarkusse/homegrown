#ifndef MEMORY_MANAGEMENT_PHYSICAL_H
#define MEMORY_MANAGEMENT_PHYSICAL_H

// TODO: Move this to x86

#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/virtual.h"

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

typedef struct {
    union {
        U8 data[LARGE_PAGE_SIZE];
        PhysicalBasePage basePages[PageTableFormat.ENTRIES];
    };
} PhysicalLargePage;

typedef struct {
    union {
        U8 data[HUGE_PAGE_SIZE];
        PhysicalBasePage
            basePages[PageTableFormat.ENTRIES * PageTableFormat.ENTRIES];
        PhysicalLargePage largePages[PageTableFormat.ENTRIES];
    };
} PhysicalHugePage;

typedef struct {
    PagedMemory_max_a memory;
    U32 usedBasePages;
    PageSize pageSize;
} PhysicalMemoryManager;

extern PhysicalMemoryManager basePMM;
extern PhysicalMemoryManager largePMM;
extern PhysicalMemoryManager hugePMM;

void initPhysicalMemoryManager(KernelMemory kernelMemory);

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageSize pageSize);
PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageSize pageSize);

void freePhysicalPage(PagedMemory page, PageSize pageSize);
void freePhysicalPages(PagedMemory_a pages, PageSize pageSize);

#endif
