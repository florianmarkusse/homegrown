#ifndef UTIL_MATHS_H
#define UTIL_MATHS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

// These operations are only defined for powers of 2 !!!
#define ALIGN_UP_EXP(val, exponent)                                            \
    (((val) + ((1U << (exponent)) - 1)) & ~((1U << (exponent)) - 1))
#define ALIGN_UP_VALUE(val, alignValue)                                        \
    (((val) + ((alignValue) - 1)) & ~((alignValue) - 1))
#define ALIGN_DOWN_EXP(val, exponent) ((val) & (~((1U << (exponent)) - 1)))
#define ALIGN_DOWN_VALUE(val, alignValue) ((val) & (~((alignValue) - 1)))
#define CEILING_DIV_EXP(val, exponent)                                         \
    (((val) + ((1U << (exponent)) - 1)) >> (exponent))
#define CEILING_DIV_VALUE(val, divisor)                                        \
    (((val) + ((divisor) - 1)) >> (__builtin_ctz(divisor)))

#define RING_RANGE_EXP(val, exponent) ((val) & ~((1U << (exponent)) - 1))
#define RING_RANGE_VALUE(val, ringSize) (((val)) & ((ringSize) - 1))
#define RING_INCREMENT(val, ringSize) (((val) + 1) & ((ringSize) - 1))
#define RING_PLUS(val, amount, ringSize) (((val) + (amount)) & ((ringSize) - 1))
#define RING_DECREMENT(val, ringSize) (((val) - 1) & ((ringSize) - 1))
#define RING_MINUS(val, amount, ringSize)                                      \
    (((val) - (amount)) & ((ringSize) - 1))

static inline U64 power(U64 base, U64 exponent) {
    U64 result = 1;

    while (exponent > 0) {
        if (exponent & 1) {
            if (result > U64_MAX / base) {
                return 0;
            }
            result *= base;
            if (exponent == 1) {
                return result;
            }
        }
        if (base > U64_MAX / base) {
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
