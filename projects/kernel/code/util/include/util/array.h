#ifndef UTIL_ARRAY_H
#define UTIL_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/macros.h"
#include "util/memory/arena.h"
#include "util/memory/memory.h"
#include "interoperation/types.h"

#define ARRAY(T)                                                               \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                          \
    }

#define DYNAMIC_ARRAY(T)                                                       \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                          \
        U64 cap;                                                          \
    }

#define MAX_LENGTH_ARRAY(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                          \
        U64 cap;                                                          \
    }

typedef struct {
    U8 *buf;
    U64 len;
    U64 cap;
} DASlice;

/**
 * Written assuming that arena bumps up! Otherwise the middle case statement
 * where we only do a times 1 alloc does not hold.
 */
__attribute((unused)) static void grow(void *slice, U64 size,
                                       U64 align, arena *a,
                                       U8 flags) {
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

#define COPY_DYNAMIC_ARRAY(newArr, oldArr, t, a)                               \
    newArr.buf = NEW(a, t, (oldArr).len);                                      \
    memcpy((newArr).buf, (oldArr).buf, (oldArr).len *SIZEOF(t));               \
    (newArr).len = (oldArr).len;                                               \
    (newArr).cap = (oldArr).len;

#define PUSH_2(s, a)                                                           \
    ({                                                                         \
        typeof(s) MACRO_VAR(s_) = (s);                                         \
        typeof(a) MACRO_VAR(a_) = (a);                                         \
        if (MACRO_VAR(s_)->len >= MACRO_VAR(s_)->cap) {                        \
            grow(MACRO_VAR(s_), SIZEOF(*MACRO_VAR(s_)->buf),                   \
                 ALIGNOF(*MACRO_VAR(s_)->buf), MACRO_VAR(a_), 0);              \
        }                                                                      \
        MACRO_VAR(s_)->buf + MACRO_VAR(s_)->len++;                             \
    })
#define PUSH_3(s, a, f)                                                        \
    ({                                                                         \
        typeof(s) MACRO_VAR(s_) = (s);                                         \
        typeof(a) MACRO_VAR(a_) = (a);                                         \
        if (MACRO_VAR(s_)->len >= MACRO_VAR(s_)->cap) {                        \
            grow(MACRO_VAR(s_), SIZEOF(*MACRO_VAR(s_)->buf),                   \
                 ALIGNOF(*MACRO_VAR(s_)->buf), MACRO_VAR(a_), f);              \
        }                                                                      \
        MACRO_VAR(s_)->buf + MACRO_VAR(s_)->len++;                             \
    })
#define PUSH_X(a, b, c, d, ...) d
#define PUSH(...) PUSH_X(__VA_ARGS__, PUSH_3, PUSH_2)(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
