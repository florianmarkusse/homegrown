#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "kernel-parameters.h"
#include "memory-management.h"

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
bool canBeUsedByOS(MemoryType type);
void initPhysicalMemoryManager(KernelMemory kernelMemory);
#endif
