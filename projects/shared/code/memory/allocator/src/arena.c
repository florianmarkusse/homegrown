#include "shared/memory/allocator/arena.h"
#include "shared/assert.h" // for ASSERT
#include "shared/types/types.h"
#include "shared/memory/allocator/macros.h"
#include "platform-abstraction/memory/manipulation.h"

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, I64 size, U64 align,
                                                  U64 count, U8 flags) {
    ASSERT((align & (align - 1)) == 0);

    U64 avail = a->end - a->curFree;
    U64 padding = -(U64)a->curFree & (align - 1);
    if (count > (avail - padding) / size) {
        if (flags & nullptr_ON_FAIL) {
            return nullptr;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    U64 total = size * count;
    U8 *p = a->curFree + padding;
    a->curFree += padding + total;

    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
}
