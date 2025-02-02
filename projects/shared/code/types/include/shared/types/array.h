#ifndef SHARED_TYPES_ARRAY_H
#define SHARED_TYPES_ARRAY_H

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

#endif
