#ifndef UTIL_ARRAY_TYPES_H
#define UTIL_ARRAY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/array.h"
#include "util/text/string.h"

typedef DYNAMIC_ARRAY(string) string_d_a;
typedef DYNAMIC_ARRAY(U64) uint64_d_a;
typedef DYNAMIC_ARRAY(bool) bool_d_a;
typedef DYNAMIC_ARRAY(U8) I8_d_a;
typedef MAX_LENGTH_ARRAY(string) string_max_a;
typedef MAX_LENGTH_ARRAY(U64) uint64_max_a;
typedef MAX_LENGTH_ARRAY(U32) uint32_max_a;
typedef MAX_LENGTH_ARRAY(U8) uint8_max_a;
typedef MAX_LENGTH_ARRAY(bool) bool_max_a;
typedef ARRAY(U8) u_I8_a;
typedef ARRAY(I8 *) I8_ptr_a;
typedef ARRAY(bool) bool_a;

#ifdef __cplusplus
}
#endif

#endif
