#include "util/memory/arena.h"
#include "util/assert.h"        // for FLO_ASSERT
#include "util/memory/macros.h" // for FLO_NULL_ON_FAIL, FLO_ZERO_MEMORY
#include "util/memory/memory.h"
#include "util/types.h"

__attribute((malloc, alloc_size(2, 4), alloc_align(3))) void *
flo_alloc(flo_arena *a, int64_t size, int64_t align, int64_t count,
          unsigned char flags) {
    FLO_ASSERT(size > 0);
    FLO_ASSERT(align > 0);
    FLO_ASSERT((align & (align - 1)) == 0);

    int64_t avail = a->end - a->beg;
    int64_t padding = -(uint64_t)a->beg & (align - 1);
    if (count > (avail - padding) / size) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        FLO_ASSERT(false);
#endif
        if (flags & FLO_NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    int64_t total = size * count;
    char *p = a->beg + padding;
    a->beg += padding + total;

    return flags & FLO_ZERO_MEMORY ? memset(p, 0, total) : p;
}

__attribute((malloc, alloc_size(3, 5), alloc_align(4))) void *
flo_copyToArena(flo_arena *arena, void *data, int64_t size, int64_t align,
                int64_t count) {
    unsigned char *copy = flo_alloc(arena, size, align, count, 0);
    memcpy(copy, data, size * count);
    return copy;
}
