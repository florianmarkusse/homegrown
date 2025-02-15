#include "shared/memory/converter.h"
#include "platform-abstraction/virtual/converter.h"

#include "shared/assert.h"
#include "shared/types/types.h"
static bool isPageSizeValid(U64 pageSize) {
    ASSERT(((pageSize) & (pageSize - 1)) == 0);
    return pageSize & AVAILABLE_PAGE_SIZES_MASK;
}

U64 smallestPageSize = 1 << __builtin_ctzl(AVAILABLE_PAGE_SIZES_MASK);

PageSizeConversion convertBytesToPages(U64 bytesPowerOfTwo) {
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
