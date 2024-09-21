#ifndef MEMORY_MANAGEMENT_DEFINITIONS_H
#define MEMORY_MANAGEMENT_DEFINITIONS_H

#include "interoperation/array.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

#define NUM_PAGE_SIZES 3
typedef enum : U64 {
    BASE_PAGE = PAGE_FRAME_SIZE,
    LARGE_PAGE = LARGE_PAGE_SIZE,
    HUGE_PAGE = HUGE_PAGE_SIZE,
} PageSize;

static PageSize pageSizes[NUM_PAGE_SIZES] = {BASE_PAGE, LARGE_PAGE, HUGE_PAGE};

#endif
