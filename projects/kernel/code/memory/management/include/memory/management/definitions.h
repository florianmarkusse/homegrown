#ifndef MEMORY_MANAGEMENT_DEFINITIONS_H
#define MEMORY_MANAGEMENT_DEFINITIONS_H

#include "interoperation/array.h"
#include "interoperation/types.h"

typedef struct {
    U64 pageStart;
    U64 numberOfPages;
} PagedMemory;

typedef MAX_LENGTH_ARRAY(PagedMemory) PagedMemory_max_a;
typedef ARRAY(PagedMemory) PagedMemory_a;

#endif
