#ifndef UTIL_MACROS_H
#define UTIL_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLO_MACRO_VAR(name) _##name##_##MACRO_VAR##__LINE__

#define FLO_STR_HELPER(x) #x
#define FLO_STRINGIFY(x) FLO_STR_HELPER(x)

#ifdef __cplusplus
}
#endif

#endif
