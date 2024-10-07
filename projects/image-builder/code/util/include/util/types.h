#ifndef UTIL_TYPES_H
#define UTIL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/array.h"
#include "util/text/string.h"
#include <stdbool.h>
#include <stdint.h>

typedef FLO_DYNAMIC_ARRAY(flo_string) flo_string_d_a;
typedef FLO_DYNAMIC_ARRAY(uint64_t) flo_uint64_d_a;
typedef FLO_DYNAMIC_ARRAY(bool) flo_bool_d_a;
typedef FLO_DYNAMIC_ARRAY(unsigned char) flo_char_d_a;
typedef FLO_MAX_LENGTH_ARRAY(flo_string) flo_string_max_a;
typedef FLO_MAX_LENGTH_ARRAY(uint64_t) flo_uint64_max_a;
typedef FLO_MAX_LENGTH_ARRAY(uint32_t) flo_uint32_max_a;
typedef FLO_MAX_LENGTH_ARRAY(bool) flo_bool_max_a;
typedef FLO_ARRAY(unsigned char) flo_char_a;
typedef FLO_ARRAY(char *) flo_char_ptr_a;
typedef FLO_ARRAY(bool) flo_bool_a;

#ifdef __cplusplus
}
#endif

#endif
