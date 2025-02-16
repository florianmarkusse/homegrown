#include "shared/memory/converter.h"
#include "platform-abstraction/virtual/converter.h"

#include "shared/assert.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(((pageSize) & (pageSize - 1)) == 0);
    return pageSize & AVAILABLE_PAGE_SIZES_MASK;
}

U64 smallestPageSize = 1 << __builtin_ctzl(AVAILABLE_PAGE_SIZES_MASK);

PageSizeConversion convertPreferredPageToAvailablePages(U64 bytesPowerOfTwo) {
    ASSERT(((bytesPowerOfTwo) & (bytesPowerOfTwo - 1)) == 0);
    if (bytesPowerOfTwo <= smallestPageSize) {
        return (PageSizeConversion){.numberOfPages = 1,
                                    .pageSize = smallestPageSize};
    }
    PageSizeConversion result =
        (PageSizeConversion){.numberOfPages = 1, .pageSize = bytesPowerOfTwo};
    while (!isPageSizeValid(result.pageSize)) {
        result.numberOfPages <<= 1;
        result.pageSize >>= 1;
    }
    return result;
}

PageSizeConversion convertBytesToPagesToMapSmartly(U64 bytes) {
    if (bytes <= smallestPageSize) {
        return (PageSizeConversion){.numberOfPages = 1,
                                    .pageSize = smallestPageSize};
    }

    PageSizeConversion result;
    for (U64 i = MEMORY_PAGE_SIZES_COUNT - 1; i != U64_MAX; i--) {
        if (pageSizes[i] / 2 <= bytes) {
            result.pageSize = pageSizes[i];
            result.numberOfPages = CEILING_DIV_VALUE(bytes, result.pageSize);
            return result;
        }
    }

    __builtin_unreachable();
}
