#include "platform-abstraction/memory/management/init.h"

#include "platform-abstraction/virtual/map.h"
#include "shared/maths/maths.h"
#include "shared/memory/management/definitions.h"
#include "x86-physical.h"
#include "x86-policy/virtual.h"
#include "x86/configuration/cpu2.h"
#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"

void initMemoryManager(KernelMemory kernelMemory) {
    initPhysicalMemoryManager(kernelMemory);
    initVirtualMemoryManager(kernelMemory);
}

void initScreenMemory(U64 screenAddress, U64 bytes) {
    PagedMemory pagedMemory = {.pageStart = screenAddress,
                               .numberOfPages =
                                   CEILING_DIV_EXP(bytes, PAGE_FRAME_SHIFT)};
    mapVirtualRegionWithFlags(screenAddress, pagedMemory, BASE_PAGE,
                              PATMapping.MAP_3);
    flushCPUCaches();
}
