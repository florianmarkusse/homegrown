#ifndef MEMORY_MANAGEMENT_PHYSICAL_H
#define MEMORY_MANAGEMENT_PHYSICAL_H

#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/descriptor.h"

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
bool canBeUsedByOS(MemoryType type);
void initPhysicalMemoryManager(KernelMemory kernelMemory);
void *allocPhysicalPages(U64 numberOfPages);
void printPhysicalMemoryManagerStatus();

#define GET_PHYSICAL(pages, t) (typeof(t))allocPhysicalPages(pages)

#endif
