#ifndef PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_VIRTUAL_H
#define PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_VIRTUAL_H

#ifdef X86_ARCHITECTURE

#include "interoperation/kernel-parameters.h"
#include "shared/memory/management/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

MappedPage getMappedPage(U64 virtual);
#else
#error "Could not match ARCHITECTURE"
#endif

#endif
