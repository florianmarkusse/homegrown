#ifndef UTIL_ARRAY_H
#define UTIL_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/macros.h"
#include "shared/allocator/arena.h"
#include <string.h>

#define FLO_ARRAY(T)                                                           \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                         \
    }

#define FLO_DYNAMIC_ARRAY(T)                                                   \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                         \
        U64 cap;                                                         \
    }

#define FLO_MAX_LENGTH_ARRAY(T)                                                \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                         \
        U64 cap;                                                         \
    }

typedef struct {
    U8 *buf;
    U64 len;
    U64 cap;
} DASlice;

/**
 * Written assuming that Arena bumps up! Otherwise the middle case statement
 * where we only do a times 1 alloc does not hold.
 */
__attribute((unused)) static void flo_grow(void *slice, U64 size,
                                           U64 align, Arena *a,
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

#define FLO_COPY_DYNAMIC_ARRAY(newArr, oldArr, t, a)                           \
    newArr.buf = NEW(a, t, (oldArr).len);                                  \
    memcpy((newArr).buf, (oldArr).buf, (oldArr).len *sizeof(t));           \
    (newArr).len = (oldArr).len;                                               \
    (newArr).cap = (oldArr).len;

#define FLO_PUSH_2(s, a)                                                       \
    ({                                                                         \
        typeof(s) FLO_MACRO_VAR(s_) = (s);                                     \
        typeof(a) FLO_MACRO_VAR(a_) = (a);                                     \
        if (FLO_MACRO_VAR(s_)->len >= FLO_MACRO_VAR(s_)->cap) {                \
            flo_grow(FLO_MACRO_VAR(s_), sizeof(*FLO_MACRO_VAR(s_)->buf),   \
                     alignof(*FLO_MACRO_VAR(s_)->buf), FLO_MACRO_VAR(a_),  \
                     0);                                                       \
        }                                                                      \
        FLO_MACRO_VAR(s_)->buf + FLO_MACRO_VAR(s_)->len++;                     \
    })
#define FLO_PUSH_3(s, a, f)                                                    \
    ({                                                                         \
        typeof(s) FLO_MACRO_VAR(s_) = (s);                                     \
        typeof(a) FLO_MACRO_VAR(a_) = (a);                                     \
        if (FLO_MACRO_VAR(s_)->len >= FLO_MACRO_VAR(s_)->cap) {                \
            flo_grow(FLO_MACRO_VAR(s_), sizeof(*FLO_MACRO_VAR(s_)->buf),   \
                     alignof(*FLO_MACRO_VAR(s_)->buf), FLO_MACRO_VAR(a_),  \
                     f);                                                       \
        }                                                                      \
        FLO_MACRO_VAR(s_)->buf + FLO_MACRO_VAR(s_)->len++;                     \
    })
#define FLO_PUSH_X(a, b, c, d, ...) d
#define FLO_PUSH(...)                                                          \
    FLO_PUSH_X(__VA_ARGS__, FLO_PUSH_3, FLO_PUSH_2)(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
