#ifndef MEMORY_MANAGEMENT_DEFINITIONS_H
#define MEMORY_MANAGEMENT_DEFINITIONS_H

#include "interoperation/array.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"
#include "text/string.h"

typedef enum {
    BASE_PAGE = 0,
    LARGE_PAGE = 1,
    HUGE_PAGE = 2,
    PAGE_TYPE_NUMS = 3
} PageType;

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

static U64 pageTypeToPageSize[PAGE_TYPE_NUMS] = {
    PAGE_FRAME_SIZE, LARGE_PAGE_SIZE, HUGE_PAGE_SIZE};

static string pageTypeToString[PAGE_TYPE_NUMS] = {
    STRING("Base page frame, 4096KiB"),
    STRING("Large page, 2MiB"),
    STRING("Huge page, 1GiB"),
};

#endif
