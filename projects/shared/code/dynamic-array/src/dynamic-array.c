#include "shared/dynamic-array/dynamic-array.h"
#include "interoperation/types.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/manipulation/manipulation.h"

typedef struct {
    U8 *buf;
    U64 len;
    U64 cap;
} DASlice;

void grow(void *slice, U64 size, U64 align, Arena *a, U8 flags) {
    DASlice *replica = (DASlice *)slice;

    if (replica->buf == NULL) {
        replica->cap = 1;
        replica->buf = alloc(a, 2 * size, align, replica->cap, flags);
    } else if (a->beg == replica->buf + size * replica->cap) {
        alloc(a, size, 1, replica->cap, flags);
    } else {
        void *data = alloc(a, 2 * size, align, replica->cap, flags);
        memcpy(data, replica->buf, size * replica->len);
        replica->buf = data;
    }

    replica->cap *= 2;
}
