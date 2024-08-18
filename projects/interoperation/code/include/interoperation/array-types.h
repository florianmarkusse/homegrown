#ifndef UTIL_ARRAY_TYPES_H
#define UTIL_ARRAY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/array.h"

typedef DYNAMIC_ARRAY(U64) U64_d_a;
typedef DYNAMIC_ARRAY(bool) bool_d_a;
typedef DYNAMIC_ARRAY(U8) U8_d_a;
typedef MAX_LENGTH_ARRAY(U64) U64_max_a;
typedef MAX_LENGTH_ARRAY(U32) U32_max_a;
typedef MAX_LENGTH_ARRAY(U8) U8_max_a;
typedef MAX_LENGTH_ARRAY(bool) bool_max_a;
typedef ARRAY(U8) U8_a;
typedef ARRAY(I8 *) I8_ptr_a;
typedef ARRAY(bool) bool_a;
typedef ARRAY(void *) void_ptr_a;

#ifdef __cplusplus
}
#endif

#endif
