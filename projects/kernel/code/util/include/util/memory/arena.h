#ifndef UTIL_MEMORY_ARENA_H
#define UTIL_MEMORY_ARENA_H

#include "util/types.h"

typedef struct {
    uint8_t *beg;
    uint8_t *end;
    uint64_t cap;
    void **jmp_buf;
} arena;

__attribute((malloc, alloc_align(3))) void *alloc(arena *a, int64_t size,
                                                  uint64_t align,
                                                  uint64_t count,
                                                  unsigned char flags);

#define NEW_2(a, t) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), 1, 0)
#define NEW_3(a, t, n) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, 0)
#define NEW_4(a, t, n, f) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, f)
#define NEW_X(a, b, c, d, e, ...) e
#define NEW(...)                                                               \
    NEW_X(__VA_ARGS__, NEW_4, NEW_3, NEW_2)                                    \
    (__VA_ARGS__)

#endif
