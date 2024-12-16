#include "platform-abstraction/memory/management/init.h"
#include "platform-abstraction/cpu.h"
#include "shared/maths/maths.h"
#include "x86/memory/pat.h"
#include "x86/memory/physical.h"
#include "x86/memory/virtual.h"

void initMemoryManager(KernelMemory kernelMemory, U64 rootMemoryMappingTable) {
    initPhysicalMemoryManager(kernelMemory);
    initVirtualMemoryManager(rootMemoryMappingTable, kernelMemory);
}

void initScreenMemory(U64 screenAddress, U64 bytes) {
    PagedMemory pagedMemory = {.pageStart = screenAddress,
                               .numberOfPages =
                                   CEILING_DIV_EXP(bytes, PAGE_FRAME_SHIFT)};
    mapVirtualRegionWithFlags(screenAddress, pagedMemory, BASE_PAGE,
                              PATMapping.MAP_3);
    flushCPUCaches();
}
