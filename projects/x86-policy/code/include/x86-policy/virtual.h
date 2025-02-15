#ifndef X86_POLICY_VIRTUAL_H
#define X86_POLICY_VIRTUAL_H

#include "efi-to-kernel/kernel-parameters.h"
#include "shared/types/types.h"
#include "x86/memory/definitions.h"

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getPhysicalAddressFrame(U64 virtualPage);
U64 getVirtualMemory(U64 size, PageSize alignValue);

MappedPage getMappedPage(U64 virt);

#endif
