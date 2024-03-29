#ifndef UTIL_MEMORY_MACROS_H
#define UTIL_MEMORY_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

#define FLO_ZERO_MEMORY 0x01
#define FLO_NULL_ON_FAIL 0x02

#define FLO_SIZEOF(x) (int64_t)sizeof(x)
#define FLO_COUNTOF(a) (FLO_SIZEOF(a) / FLO_SIZEOF(*(a)))
#define FLO_LENGTHOF(s) (FLO_COUNTOF(s) - 1)
#define FLO_ALIGNOF(t) (_Alignof(t))

#ifdef __cplusplus
}
#endif

#endif
