#ifndef UTIL_MEMORY_ARENA_H
#define UTIL_MEMORY_ARENA_H

#include "util/types.h"

typedef struct {
    char *beg;
    char *end;
    int64_t cap;
    void **jmp_buf;
} flo_arena;

__attribute((malloc, alloc_size(2, 4), alloc_align(3))) void *
flo_alloc(flo_arena *a, int64_t size, int64_t align, int64_t count,
          unsigned char flags);

__attribute((malloc, alloc_size(3, 5), alloc_align(4))) void *
flo_copyToArena(flo_arena *arena, void *data, int64_t size, int64_t align,
                int64_t count);

#define FLO_NEW_2(a, t) (t *)flo_alloc(a, FLO_SIZEOF(t), FLO_ALIGNOF(t), 1, 0)
#define FLO_NEW_3(a, t, n)                                                     \
    (t *)flo_alloc(a, FLO_SIZEOF(t), FLO_ALIGNOF(t), n, 0)
#define FLO_NEW_4(a, t, n, f)                                                  \
    (t *)flo_alloc(a, FLO_SIZEOF(t), FLO_ALIGNOF(t), n, f)
#define FLO_NEW_X(a, b, c, d, e, ...) e
#define FLO_NEW(...)                                                           \
    FLO_NEW_X(__VA_ARGS__, FLO_NEW_4, FLO_NEW_3, FLO_NEW_2)                    \
    (__VA_ARGS__)

#endif
