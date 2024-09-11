#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define NULL ((void *)0)

#if !defined(__INT8_TYPE__) || !defined(__UINT8_TYPE__) ||                     \
    !defined(__INT16_TYPE__) || !defined(__UINT16_TYPE__) ||                   \
    !defined(__INT32_TYPE__) || !defined(__UINT32_TYPE__) ||                   \
    !defined(__INT64_TYPE__) || !defined(__UINT64_TYPE__) ||                   \
    (!defined(__INT8_C) && !defined(__INT8_C_SUFFIX__)) ||                     \
    (!defined(__INT16_C) && !defined(__INT16_C_SUFFIX__)) ||                   \
    (!defined(__INT32_C) && !defined(__INT32_C_SUFFIX__)) ||                   \
    (!defined(__INT64_C) && !defined(__INT64_C_SUFFIX__)) ||                   \
    (!defined(__UINT8_C) && !defined(__UINT8_C_SUFFIX__)) ||                   \
    (!defined(__UINT16_C) && !defined(__UINT16_C_SUFFIX__)) ||                 \
    (!defined(__UINT32_C) && !defined(__UINT32_C_SUFFIX__)) ||                 \
    (!defined(__UINT64_C) && !defined(__UINT64_C_SUFFIX__))
#error "Compiler does not provide fixed-size integer macros."
#endif

#define JOIN(_a, _b) JOIN_LITERALS(_a, _b)
#define JOIN_LITERALS(_a, _b) _a##_b

#define I8_C(_v) JOIN(_v, __INT8_C_SUFFIX__)
#define U8_C(_v) JOIN(_v, __UINT8_C_SUFFIX__)
#define I16_C(_v) JOIN(_v, __INT16_C_SUFFIX__)
#define U16_C(_v) JOIN(_v, __UINT16_C_SUFFIX__)
#define I32_C(_v) JOIN(_v, __INT32_C_SUFFIX__)
#define U32_C(_v) JOIN(_v, __UINT32_C_SUFFIX__)
#define I64_C(_v) JOIN(_v, __INT64_C_SUFFIX__)
#define U64_C(_v) JOIN(_v, __UINT64_C_SUFFIX__)

#define I8_MIN I8_C(-128)
#define I16_MIN I16_C(-32767 - 1)
#define I32_MIN I32_C(-2147483647 - 1)
#define I64_MIN I64_C(-9223372036854775807 - 1)

#define I8_MAX (127)
#define I16_MAX (32767)
#define I32_MAX (2147483647)
#define I64_MAX (9223372036854775807)

#define U8_MAX U8_C(0xFF)
#define U16_MAX U16_C(0xFFFF)
#define U32_MAX U32_C(0xFFFFFFFF)
#define U64_MAX U64_C(0xFFFFFFFFFFFFFFFF)

typedef __INT8_TYPE__ I8;
typedef __UINT8_TYPE__ U8;
typedef __INT16_TYPE__ I16;
typedef __UINT16_TYPE__ U16;
typedef __INT32_TYPE__ I32;
typedef __UINT32_TYPE__ U32;
typedef __INT64_TYPE__ I64;
typedef __UINT64_TYPE__ U64;
typedef __INTPTR_TYPE__ ISize;
typedef __UINTPTR_TYPE__ USize;

#define F32_MIN (-3.402823466e+38F)
#define F32_MAX 3.402823466e+38F
#define F32_EPSILON 1.192092896e-07F
#define F32_MIN_POS 1.175494351e-38F

#define F64_MIN (-1.7976931348623158e+308)
#define F64_MAX 1.7976931348623158e+308
#define F64_EPSILON 2.2204460492503131e-16
#define F64_MIN_POS 2.2250738585072014e-308

// Assuming 80-bit extended precision for long double
// If u need more, what are you doing?
// It does mess up with the epsilon on other machines but do you ever need to
// use that?
#define F128_MIN (-1.18973149535723176502e+4932L)  // Minimum negative value
#define F128_MAX 1.18973149535723176502e+4932L     // Maximum positive value
#define F128_EPSILON 1.08420217248550443401e-19L   // Epsilon value
#define F128_MIN_POS 3.36210314311209350626e-4932L // Minimum positive value

typedef float F32;
typedef double F64;
typedef long double F128;

#define TYPED_CONSTANT(x, value)                                               \
    _Generic((x),                                                              \
        I8: I8_C(value),                                                       \
        U8: U8_C(value),                                                       \
        I16: I16_C(value),                                                     \
        U16: U16_C(value),                                                     \
        I32: I32_C(value),                                                     \
        U32: U32_C(value),                                                     \
        I64: I64_C(value),                                                     \
        U64: U64_C(value),                                                     \
        default: "unknown")

#define MAX_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: I8_MAX,                                                            \
        U8: U8_MAX,                                                            \
        I16: I16_MAX,                                                          \
        U16: U16_MAX,                                                          \
        I32: I32_MAX,                                                          \
        U32: U32_MAX,                                                          \
        I64: I64_MAX,                                                          \
        U64: U64_MAX,                                                          \
        F32: F32_MAX,                                                          \
        F64: F64_MAX,                                                          \
        F128: F128_MAX,                                                        \
        default: "unknown")

#define MIN_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: I8_MIN,                                                            \
        I16: I16_MIN,                                                          \
        I32: I32_MIN,                                                          \
        I64: I64_MIN,                                                          \
        F32: F32_MIN,                                                          \
        F64: F64_MIN,                                                          \
        F128: F128_MIN,                                                        \
        default: "unknown")

#define EPSILON_VALUE(x)                                                       \
    _Generic((x),                                                              \
        F32: F32_EPSILON,                                                      \
        F64: F64_EPSILON,                                                      \
        F128: F128_EPSILON,                                                    \
        default: "unknown")

#define MIN_POS_VALUE(x)                                                       \
    _Generic((x),                                                              \
        F32: F32_MIN_POS,                                                      \
        F64: F64_MIN_POS,                                                      \
        F128: F128_MIN_POS,                                                    \
        default: "unknown")

#ifdef __cplusplus
}
#endif

#endif
