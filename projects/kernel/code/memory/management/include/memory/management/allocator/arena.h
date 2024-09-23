#ifndef MEMORY_MANAGEMENT_ARENA_ARENA_H
#define MEMORY_MANAGEMENT_ARENA_ARENA_H

#include "interoperation/types.h"
#include "memory/management/allocator/macros.h"
#include "util/macros.h"

typedef struct {
    U8 *curFree;
    U8 *beg;
    U8 *end;
    void **jmp_buf;
} Arena;

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, I64 size, U64 align,
                                                  U64 count, U8 flags);

#define NEW_2(a, t) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), 1, 0)
#define NEW_3(a, t, n) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, 0)
#define NEW_4(a, t, n, f) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, f)
#define NEW_X(a, b, c, d, e, ...) e
#define NEW(...)                                                               \
    NEW_X(__VA_ARGS__, NEW_4, NEW_3, NEW_2)                                    \
    (__VA_ARGS__)

#endif
