#ifndef SHARED_MACROS_H
#define SHARED_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#define MACRO_VAR(name) _##name##_##MACRO_VAR##__LINE__

#define STR_HELPER(x) #x
#define STRINGIFY(x) STR_HELPER(x)

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#ifdef __cplusplus
}
#endif

#endif
