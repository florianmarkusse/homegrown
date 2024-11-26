#ifndef SHARED_TYPES_ARRAY_H
#define SHARED_TYPES_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/types/types.h"

#define ARRAY(T)                                                               \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
    }

#define DYNAMIC_ARRAY(T)                                                       \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
        U64 cap;                                                               \
    }

#define MAX_LENGTH_ARRAY(T)                                                    \
    struct {                                                                   \
        T *buf;                                                                \
        U64 len;                                                               \
        U64 cap;                                                               \
    }

#ifdef __cplusplus
}
#endif

#endif
