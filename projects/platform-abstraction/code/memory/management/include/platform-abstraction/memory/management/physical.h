#ifndef PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_PHYSICAL_H
#define PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_PHYSICAL_H

#ifdef X86_ARCHITECTURE

#include "interoperation/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"

void initPhysicalMemoryManager(KernelMemory kernelMemory);

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageSize pageSize);
PagedMemory_a allocPhysicalPages(PagedMemory_a pages, PageSize pageSize);

void freePhysicalPage(PagedMemory page, PageSize pageSize);
void freePhysicalPages(PagedMemory_a pages, PageSize pageSize);
#else
#error "Could not match ARCHITECTURE"
#endif

#endif
