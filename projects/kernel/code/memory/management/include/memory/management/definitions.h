#ifndef MEMORY_MANAGEMENT_DEFINITIONS_H
#define MEMORY_MANAGEMENT_DEFINITIONS_H

#include "interoperation/array.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"
#include "text/string.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

typedef enum : U64 {
    BASE_PAGE = PAGE_FRAME_SIZE,
    LARGE_PAGE = LARGE_PAGE_SIZE,
    HUGE_PAGE = HUGE_PAGE_SIZE,
    PAGE_TYPE_NUMS = 3
} PageType;

#define BIGGER_PAGE_SIZE(pageType) ((pageType) << PAGE_TABLE_SHIFT)
#define SMALLER_PAGE_SIZE(pageType) ((pageType) >> PAGE_TABLE_SHIFT)

static U8 pageTypeToDepth[PAGE_TYPE_NUMS] = {4, 3, 2};

static string pageTypeToString[PAGE_TYPE_NUMS] = {
    STRING("Base page frame, 4096KiB"),
    STRING("Large page, 2MiB"),
    STRING("Huge page, 1GiB"),
};

#endif
