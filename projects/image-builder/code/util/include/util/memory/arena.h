#ifndef UTIL_MEMORY_ARENA_H
#define UTIL_MEMORY_ARENA_H

#include "macros.h" // for alignof, sizeof
#include <stddef.h> // for ptrdiff_t

typedef struct {
    char *beg;
    char *end;
    ptrdiff_t cap;
    void **jmp_buf;
} flo_arena;

__attribute((malloc, alloc_align(3))) void *
flo_alloc(flo_arena *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count,
          unsigned char flags);

__attribute((malloc, alloc_align(4))) void *
flo_copyToArena(flo_arena *arena, void *data, ptrdiff_t size, ptrdiff_t align,
                ptrdiff_t count);

#define FLO_NEW_2(a, t) (t *)flo_alloc(a, sizeof(t), alignof(t), 1, 0)
#define FLO_NEW_3(a, t, n)                                                     \
    (t *)flo_alloc(a, sizeof(t), alignof(t), n, 0)
#define FLO_NEW_4(a, t, n, f)                                                  \
    (t *)flo_alloc(a, sizeof(t), alignof(t), n, f)
#define FLO_NEW_X(a, b, c, d, e, ...) e
#define NEW(...)                                                           \
    FLO_NEW_X(__VA_ARGS__, FLO_NEW_4, FLO_NEW_3, FLO_NEW_2)                    \
    (__VA_ARGS__)

#endif
