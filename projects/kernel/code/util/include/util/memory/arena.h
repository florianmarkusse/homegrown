#ifndef UTIL_MEMORY_ARENA_H
#define UTIL_MEMORY_ARENA_H

#include "types.h"

typedef struct {
    U8 *beg;
    U8 *end;
    U64 cap;
    void **jmp_buf;
} arena;

__attribute((malloc, alloc_align(3))) void *
alloc(arena *a, I64 size, U64 align, U64 count, unsigned char flags);

#define NEW_2(a, t) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), 1, 0)
#define NEW_3(a, t, n) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, 0)
#define NEW_4(a, t, n, f) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, f)
#define NEW_X(a, b, c, d, e, ...) e
#define NEW(...)                                                               \
    NEW_X(__VA_ARGS__, NEW_4, NEW_3, NEW_2)                                    \
    (__VA_ARGS__)

#endif
