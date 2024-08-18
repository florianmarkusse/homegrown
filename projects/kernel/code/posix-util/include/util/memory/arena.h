#ifndef UTIL_MEMORY_ARENA_H
#define UTIL_MEMORY_ARENA_H

#include "macros.h" // for ALIGNOF, SIZEOF
#include <stddef.h> // for ptrdiff_t

typedef struct {
    char *beg;
    char *end;
    ptrdiff_t cap;
    void **jmp_buf;
} Arena;

__attribute((malloc, alloc_align(3))) void *alloc(Arena *a, ptrdiff_t size,
                                                  ptrdiff_t align,
                                                  ptrdiff_t count,
                                                  unsigned char flags);

__attribute((malloc, alloc_align(4))) void *
copyToArena(Arena *arena, void *data, ptrdiff_t size, ptrdiff_t align,
            ptrdiff_t count);

#define NEW_2(a, t) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), 1, 0)
#define NEW_3(a, t, n) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, 0)
#define NEW_4(a, t, n, f) (t *)alloc(a, SIZEOF(t), ALIGNOF(t), n, f)
#define NEW_X(a, b, c, d, e, ...) e
#define NEW(...)                                                               \
    NEW_X(__VA_ARGS__, NEW_4, NEW_3, NEW_2)                                    \
    (__VA_ARGS__)

#endif
