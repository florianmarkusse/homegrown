#include "memory/management/allocator/arena.h"
#include "interoperation/types.h"
#include "memory/management/allocator/macros.h"
#include "memory/manipulation/manipulation.h"
#include "util/assert.h" // for ASSERT

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, I64 size, U64 align,
                                                  U64 count, U8 flags) {
    ASSERT((align & (align - 1)) == 0);

    U64 avail = a->end - a->curFree;
    U64 padding = -(U64)a->curFree & (align - 1);
    if (count > (avail - padding) / size) {
        if (flags & NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    U64 total = size * count;
    U8 *p = a->curFree + padding;
    a->curFree += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}
