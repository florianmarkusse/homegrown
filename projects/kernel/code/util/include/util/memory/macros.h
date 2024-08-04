#ifndef UTIL_MEMORY_MACROS_H
#define UTIL_MEMORY_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

#define ZERO_MEMORY 0x01
#define NULL_ON_FAIL 0x02

#define SIZEOF(x) (I64)sizeof(x)
#define COUNTOF(a) (SIZEOF(a) / SIZEOF(*(a)))
#define LENGTHOF(s) (COUNTOF(s) - 1)
#define ALIGNOF(t) (_Alignof(t))

#ifdef __cplusplus
}
#endif

#endif
