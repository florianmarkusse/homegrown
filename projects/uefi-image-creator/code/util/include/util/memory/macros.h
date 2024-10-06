#ifndef UTIL_MEMORY_MACROS_H
#define UTIL_MEMORY_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

static constexpr auto FLO_ZERO_MEMORY = 0x01;
static constexpr auto FLO_NULL_ON_FAIL = 0x02;

#define FLO_COUNTOF(a) (sizeof(a) / sizeof(*(a)))
#define FLO_LENGTHOF(s) (FLO_COUNTOF(s) - 1)

#ifdef __cplusplus
}
#endif

#endif
