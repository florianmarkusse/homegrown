#ifndef X86_MEMORY_PAGE_H
#define X86_MEMORY_PAGE_H

#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"

static constexpr auto NUM_PAGE_SIZES = 3;
typedef enum : U64 {
    BASE_PAGE = PAGE_FRAME_SIZE,
    LARGE_PAGE = LARGE_PAGE_SIZE,
    HUGE_PAGE = HUGE_PAGE_SIZE,
} PageSize;

extern PageSize pageSizes[NUM_PAGE_SIZES];

#endif
