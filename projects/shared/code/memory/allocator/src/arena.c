#include "shared/memory/allocator/arena.h"
#include "abstraction/memory/manipulation.h"
#include "shared/assert.h" // for ASSERT
#include "shared/memory/allocator/macros.h"
#include "shared/types/types.h"

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, U64 size, U64 align,
                                                  U64 count, U8 flags) {
    ASSERT((align & (align - 1)) == 0);

    U64 avail = a->end - a->curFree;
    U64 padding = -(U64)a->curFree & (align - 1);
    if (count > (avail - padding) / size) {
        if (flags & NULLPTR_ON_FAIL) {
            return nullptr;
        }
        __builtin_longjmp(a->jmp_buf, 1);
    }
    U64 total = size * count;
    U8 *p = a->curFree + padding;
    a->curFree += padding + total;

#ifdef POSIX_ENVIRONMENT
    // Memory is already zeroed on this platform for security reasons
    return p;
#else
    return flags & ZERO_MEMORY ? memset(p, 0, total) : p;
#endif
}
