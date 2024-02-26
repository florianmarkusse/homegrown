#ifndef UTIL_RING_BUFFER_H
#define UTIL_RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

#define FLO_RING_BUFFER(T)                                                     \
    struct {                                                                   \
        T *buf;                                                                \
        int64_t len;                                                           \
        int64_t current;                                                       \
    }

#define FLO_CREATE_RING_BUFFER(name, _struct, buffer, _len)                    \
    static_assert(!((_len) & ((_len) - 1)));                                   \
    _struct name = {.buf = (buffer), .len = (_len), .current = 0};

typedef FLO_RING_BUFFER(uint8_t) flo_uint8_rb;

#ifdef __cplusplus
}
#endif

#endif
