#ifndef SHARED_MEMORY_CONVERTER_H
#define SHARED_MEMORY_CONVERTER_H

#include "shared/types/types.h"

typedef struct {
    U64 numberOfPages;
    U64 pageSize;
} PageSizeConversion;

PageSizeConversion convertPreferredPageToAvailablePages(U64 bytesPowerOfTwo);

// Converts the given bytes to a sensible conversion of available page sizes.
// I.e., If you pass (2 MiB - 1 KiB), it will return 1 page of size 2 MiB.
PageSizeConversion convertBytesToPagesToMapSmartly(U64 bytesPowerOfTwo);

#endif
