#ifndef MEMORY_MANAGEMENT_VIRTUAL_H
#define MEMORY_MANAGEMENT_VIRTUAL_H

#include "interoperation/kernel-parameters.h"
#include "interoperation/types.h"
#include "memory/management/definitions.h"

typedef struct {
    U64 address : 48;
} PhysicalAddress;

typedef struct {
    union {
        U64 value;
        struct {
            U64 present : 1;
            U64 writable : 1;
            U64 userAccessible : 1;
            U64 writeThrough : 1;
            U64 disableCache : 1;
            U64 accessed : 1;
            U64 dirty : 1;
            U64 extendedSize : 1;
            U64 global : 1;
            U64 available_9 : 1;
            U64 available_10 : 1;
            U64 available_11 : 1;
            U64 level_1 : 9;
            U64 level_2 : 9;
            U64 level_3 : 9;
            U64 level_4 : 9;
            U64 available_52 : 1;
            U64 available_53 : 1;
            U64 available_54 : 1;
            U64 available_55 : 1;
            U64 available_56 : 1;
            U64 available_57 : 1;
            U64 available_58 : 1;
            U64 available_59 : 1;
            U64 available_60 : 1;
            U64 available_61 : 1;
            U64 available_62 : 1;
            U64 noExecute : 1;
        };
    };
} VirtualEntry;

static inline U64 getPhysicalAddress(U64 virtualPage) {
    return virtualPage & 0x000FFFFFFFFF000;
}

typedef struct {
    union {
        U64 value;
        VirtualEntry entry;
    };
} PageEntry;

void appendVirtualMemoryManagerStatus();

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory);

U64 getVirtualMemory(U64 size, PageSize alignValue);
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags);
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType);

typedef struct {
    VirtualEntry entry;
    PageSize pageSize;
} MappedPage;
MappedPage getMappedPage(U64 virtual);

#endif
