#ifndef UTIL_MACROS_H
#define UTIL_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

#define MACRO_VAR(name) _##name##_##MACRO_VAR##__LINE__

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
#define LENGTHOF(s) (COUNTOF(s) - 1)

#ifdef __cplusplus
}
#endif

#endif
