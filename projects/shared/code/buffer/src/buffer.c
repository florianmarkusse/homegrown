#include "shared/buffer/buffer.h"

#include "platform-abstraction/memory/manipulation.h"
#include "shared/types/types.h"

void appendToSimpleBuffer(string data, U8_d_a *array, Arena *perm) {
    if (array->len + data.len > array->cap) {
        U64 newCap = (array->len + data.len) * 2;
        if (array->buf == NULL) {
            array->cap = data.len;
            array->buf = alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
        } else if (perm->end == (U8 *)(array->buf - array->cap)) {
            alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
        } else {
            void *buf = alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
            memcpy(buf, array->buf, array->len);
            array->buf = buf;
        }

        array->cap = newCap;
    }
    memcpy(array->buf + array->len, data.buf, data.len);
    array->len += data.len;
}
