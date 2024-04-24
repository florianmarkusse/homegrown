#ifndef UTIL_MACROS_H
#define UTIL_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#define MACRO_VAR(name) _##name##_##MACRO_VAR##__LINE__

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)

#ifdef __cplusplus
}
#endif

#endif
