#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

#define RING_RANGE(val, ringSize) (((val)) & ((ringSize) - 1))
#define RING_INCREMENT(val, ringSize) (((val) + 1) & ((ringSize) - 1))
#define RING_PLUS(val, amount, ringSize) (((val) + (amount)) & ((ringSize) - 1))
#define RING_DECREMENT(val, ringSize) (((val) - 1) & ((ringSize) - 1))
#define RING_MINUS(val, amount, ringSize)                                      \
    (((val) - (amount)) & ((ringSize) - 1))

__attribute__((unused)) static inline U64 power(U64 base,
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
