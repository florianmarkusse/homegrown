#ifndef UTIL_TYPES_H
#define UTIL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/array.h"
#include "util/text/string.h"
#include <stdbool.h>
#include <stdint.h>

typedef FLO_DYNAMIC_ARRAY(string) string_d_a;
typedef FLO_DYNAMIC_ARRAY(U64) flo_uint64_d_a;
typedef FLO_DYNAMIC_ARRAY(bool) flo_bool_d_a;
typedef FLO_DYNAMIC_ARRAY(U8) flo_U8_d_a;
typedef FLO_MAX_LENGTH_ARRAY(string) string_max_a;
typedef FLO_MAX_LENGTH_ARRAY(U64) flo_uint64_max_a;
typedef FLO_MAX_LENGTH_ARRAY(U32) flo_uint32_max_a;
typedef FLO_MAX_LENGTH_ARRAY(bool) flo_bool_max_a;
typedef FLO_ARRAY(U8) flo_U8_a;
typedef FLO_ARRAY(U8 *) flo_U8_ptr_a;
typedef FLO_ARRAY(bool) flo_bool_a;

#ifdef __cplusplus
}
#endif

#endif
