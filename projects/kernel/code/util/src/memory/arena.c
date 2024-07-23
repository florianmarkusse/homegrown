#include "util/memory/arena.h"
#include "util/assert.h"        // for ASSERT
#include "util/memory/macros.h" // for NULL_ON_FAIL, ZERO_MEMORY
#include "util/memory/memory.h"
#include "types.h"

__attribute((malloc, alloc_align(3))) void *alloc(arena *a, I64 size,
                                                  U64 align,
                                                  U64 count,
                                                  unsigned char flags) {
    ASSERT((align & (align - 1)) == 0);

    U64 avail = a->end - a->beg;
    U64 padding = -(U64)a->beg & (align - 1);
    if (count > (avail - padding) / size) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        ASSERT(false);
#endif
        if (flags & NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    U64 total = size * count;
    U8 *p = a->beg + padding;
    a->beg += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}
