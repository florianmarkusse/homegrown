#ifndef MEMORY_MANAGEMENT_PHYSICAL_H
#define MEMORY_MANAGEMENT_PHYSICAL_H

#include "interoperation/array.h"
#include "interoperation/kernel-parameters.h"
#include "interoperation/memory/definitions.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} FreeMemory;

typedef MAX_LENGTH_ARRAY(FreeMemory) FreeMemory_max_a;
typedef ARRAY(FreeMemory) FreeMemory_a;

// NOTE this can likely be moved up into a "definitions" file that is shared
// among the memory modules.
typedef enum {
    BASE_PAGE = 0,
    LARGE_PAGE = 1,
    HUGE_PAGE = 2,
    PAGE_TYPE_NUMS = 3
} PageType;

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

typedef struct {
    union {
        U8 data[LARGE_PAGE_SIZE];
        PhysicalBasePage basePages[PAGE_TABLE_ENTRIES];
    };
} PhysicalLargePage;

typedef struct {
    union {
        U8 data[HUGE_PAGE_SIZE];
        PhysicalBasePage basePages[PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRIES];
        PhysicalLargePage largePages[PAGE_TABLE_ENTRIES];
    };
} PhysicalHugePage;

void initPhysicalMemoryManager(KernelMemory kernelMemory);

U64 allocContiguousPhysicalPages(U64 numberOfPages, PageType pageType);
FreeMemory_a allocPhysicalPages(FreeMemory_a pages, PageType pageType);

void freePhysicalPage(FreeMemory page, PageType pageType);
void freePhysicalPages(FreeMemory_a pages, PageType pageType);

void printPhysicalMemoryManagerStatus();

#define GET_PHYSICAL(pages, t) (typeof(t))allocPhysicalPages(pages)

#endif
