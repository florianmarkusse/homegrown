#include "util/memory/arena.h"
#include "util/assert.h"        // for ASSERT
#include "util/memory/macros.h" // for NULL_ON_FAIL, ZERO_MEMORY
#include "util/memory/memory.h"
#include "util/types.h"

__attribute((malloc, alloc_align(3))) void *alloc(arena *a, int64_t size,
                                                  uint64_t align,
                                                  uint64_t count,
                                                  unsigned char flags) {
    ASSERT((align & (align - 1)) == 0);

    uint64_t avail = a->end - a->beg;
    uint64_t padding = -(uint64_t)a->beg & (align - 1);
    if (count > (avail - padding) / size) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        ASSERT(false);
#endif
        if (flags & NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    uint64_t total = size * count;
    uint8_t *p = a->beg + padding;
    a->beg += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}
