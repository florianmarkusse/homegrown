#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

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

#define INT8_MIN I8_C(-128)
#define INT16_MIN I16_C(-32767 - 1)
#define INT32_MIN I32_C(-2147483647 - 1)
#define INT64_MIN I64_C(-9223372036854775807 - 1)

#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807)

#define UINT8_MAX U8_C(0xFF)
#define UINT16_MAX U16_C(0xFFFF)
#define UINT32_MAX U32_C(0xFFFFFFFF)
#define UINT64_MAX U64_C(0xFFFFFFFFFFFFFFFF)

#define MAX_VALUE(x)                                                           \
    _Generic((x),                                                              \
        I8: INT8_MAX,                                                      \
        U8: UINT8_MAX,                                                    \
        I16: INT16_MAX,                                                    \
        U16: UINT16_MAX,                                                  \
        I32: INT32_MAX,                                                    \
        U32: UINT32_MAX,                                                  \
        I64: INT64_MAX,                                                    \
        U64: UINT64_MAX,                                                  \
        default: "unknown")

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

#define NULL ((void *)0)

#ifdef __cplusplus
}
#endif

#endif
