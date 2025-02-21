#ifndef ABSTRACTION_MEMORY_MANAGEMENT_INIT_H
#define ABSTRACTION_MEMORY_MANAGEMENT_INIT_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/types.h"

void initMemoryManager(KernelMemory kernelMemory);
void initScreenMemory(U64 screenAddress, U64 bytes);

#endif
