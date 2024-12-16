#ifndef PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_INIT_H
#define PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_INIT_H

#include "interoperation/kernel-parameters.h"
#include "shared/types/types.h"

void initMemoryManager(KernelMemory kernelMemory, U64 rootMemoryMappingTable);
void initScreenMemory(U64 screenAddress, U64 bytes);

#endif
