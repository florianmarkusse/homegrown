#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/types.h"

typedef struct {
    U64 numberOfPages;
    U64 pageSize;
} PageSizeConversion;

PageSizeConversion convertBytesToPages(U64 bytesPowerOfTwo);

#endif
