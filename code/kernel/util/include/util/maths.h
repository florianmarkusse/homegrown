#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

#define FLO_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FLO_MAX(a, b) (((a) > (b)) ? (a) : (b))

__attribute__((unused)) static inline uint64_t flo_power(uint64_t base,
                                                         uint64_t exponent) {
    uint64_t result = 1;

    while (exponent > 0) {
        if (exponent & 1) {
            if (result > UINT64_MAX / base) {
                return 0;
            }
            result *= base;
            if (exponent == 1) {
                return result;
            }
        }
        if (base > UINT64_MAX / base) {
            return 0;
        }
        base *= base;
        exponent >>= 1;
    }

    return result;
}

#ifdef __cplusplus
}
#endif

#endif
