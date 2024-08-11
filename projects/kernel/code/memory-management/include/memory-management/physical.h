#ifndef MEMORY_MANAGEMENT_PHYSICAL_H
#define MEMORY_MANAGEMENT_PHYSICAL_H

#include "interoperation/kernel-parameters.h"
#include "util/array.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} FreeMemory;

typedef MAX_LENGTH_ARRAY(FreeMemory) FreeMemory_max_a;
typedef ARRAY(FreeMemory) FreeMemory_a;

void initPhysicalMemoryManager(KernelMemory kernelMemory);

U64 allocContiguousBasePhysicalPages(U64 numberOfPages);
FreeMemory_a allocBasePhysicalPages(FreeMemory_a pages);
void freeBasePhysicalPages(FreeMemory_a pages);

void printPhysicalMemoryManagerStatus();

#define GET_PHYSICAL(pages, t) (typeof(t))allocPhysicalPages(pages)

#endif
