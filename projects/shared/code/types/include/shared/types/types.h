#ifndef SHARED_TYPES_TYPES_H
#define SHARED_TYPES_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

// Posix issues when you use a constexpr her
/* NOLINTNEXTLINE */
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

static constexpr I8 I8_MIN = -128;
static constexpr I16 I16_MIN = -32767 - 1;
static constexpr I32 I32_MIN = -2147483647 - 1;
static constexpr I64 I64_MIN = -9223372036854775807 - 1;

static constexpr I8 I8_MAX = (127);
static constexpr I16 I16_MAX = (32767);
static constexpr I32 I32_MAX = (2147483647);
static constexpr I64 I64_MAX = (9223372036854775807);

static constexpr U8 U8_MAX = 0xFF;
static constexpr U16 U16_MAX = 0xFFFF;
static constexpr U32 U32_MAX = 0xFFFFFFFF;
static constexpr U64 U64_MAX = 0xFFFFFFFFFFFFFFFF;

typedef float F32;
typedef double F64;
/*typedef long double F128;*/

static constexpr F32 F32_MIN = (-3.402823466e+38F);
static constexpr F32 F32_MAX = 3.402823466e+38F;
static constexpr F32 F32_EPSILON = 1.192092896e-07F;
static constexpr F32 F32_MIN_POS = 1.175494351e-38F;

static constexpr auto F64_MIN = (-1.7976931348623158e+308);
static constexpr auto F64_MAX = 1.7976931348623158e+308;
static constexpr auto F64_EPSILON = 2.2204460492503131e-16;
static constexpr auto F64_MIN_POS = 2.2250738585072014e-308;

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
        default: "unknown")

#define MIN_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: I8_MIN,                                                            \
        I16: I16_MIN,                                                          \
        I32: I32_MIN,                                                          \
        I64: I64_MIN,                                                          \
        F32: F32_MIN,                                                          \
        F64: F64_MIN,                                                          \
        default: "unknown")

#define EPSILON_VALUE(x)                                                       \
    _Generic((x), F32: F32_EPSILON, F64: F64_EPSILON, default: "unknown")

#define MIN_POS_VALUE(x)                                                       \
    _Generic((x), F32: F32_MIN_POS, F64: F64_MIN_POS, default: "unknown")

static constexpr auto BITS_PER_BYTE = 8;

#ifdef __cplusplus
}
#endif

#endif
