#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FLO_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FLO_MAX(a, b) (((a) > (b)) ? (a) : (b))

__attribute__((unused)) static inline U64 flo_power(U64 base,
                                                         U64 exponent) {
    U64 result = 1;

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
