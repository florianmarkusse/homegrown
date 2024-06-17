#ifndef UTIL_TYPES_H
#define UTIL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define NULL 0

#define INT64_C(c) c##L
#define UINT64_C(c) c##UL

/* Minimum of signed integral types.  */
#define INT8_MIN (-128)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-INT64_C(9223372036854775807) - 1)
/* Maximum of signed integral types.  */
#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
#define UINT8_MAX 0xFF
#define UINT16_MAX 0xFFFF
#define UINT32_MAX 0xFFFFFFFF
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long int int64_t;

#ifdef __cplusplus
}
#endif

#endif
