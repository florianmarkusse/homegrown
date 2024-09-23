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

typedef struct {
    U64 start;
    U64 end;
} VirtualRegion;

typedef struct {
    U64 pages[PAGE_TABLE_ENTRIES];
} VirtualPageTable;

extern VirtualPageTable *level4PageTable;

extern VirtualRegion higherHalfRegion;
extern VirtualRegion lowerHalfRegion; // Start is set in the init function.

typedef enum {
    PAT_UNCACHABLE_UC = 0x0,
    PAT_WRITE_COMBINGING_WC = 0x1,
    PAT_RESERVED_2 = 0x2,
    PAT_RESERVED_3 = 0x3,
    PAT_WRITE_THROUGH_WT = 0x4,
    PAT_WRITE_PROTECTED_WP = 0x5,
    PAT_WRITE_BACK_WB = 0x6,
    PAT_UNCACHED_UC_ = 0x7,
    PAT_NUMS
} PATEncoding;

#define PAT_LOCATION 0x277

typedef struct {
    U8 pat : 3;
    U8 reserved : 5;
} PATEntry;
typedef struct {
    union {
        PATEntry pats[8];
        U64 value;
    };
} PAT;

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
