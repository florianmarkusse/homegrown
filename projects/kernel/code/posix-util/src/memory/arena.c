#include "util/memory/arena.h"
#include "util/assert.h"        // for ASSERT
#include "util/memory/macros.h" // for NULL_ON_FAIL, ZERO_MEMORY
#include <stdint.h>             // for uintptr_t
#include <string.h>             // for memcpy, memset

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, ptrdiff_t size,
                                                  ptrdiff_t align,
                                                  ptrdiff_t count,
                                                  unsigned char flags) {
    ASSERT(size > 0);
    ASSERT(align > 0);
    ASSERT((align & (align - 1)) == 0);

    ptrdiff_t avail = a->end - a->beg;
    ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
    if (count > (avail - padding) / size) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        ASSERT(false);
#endif
        if (flags & NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    ptrdiff_t total = size * count;
    char *p = a->beg + padding;
    a->beg += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}

__attribute((malloc, alloc_align(4))) void *
copyToArena(Arena *arena, void *data, ptrdiff_t size, ptrdiff_t align,
            ptrdiff_t count) {
    unsigned char *copy = alloc(arena, size, align, count, 0);
    memcpy(copy, data, size * count);
    return copy;
}
