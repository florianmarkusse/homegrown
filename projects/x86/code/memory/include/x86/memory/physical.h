#ifndef X86_MEMORY_PHYSICAL_H
#define X86_MEMORY_PHYSICAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

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
