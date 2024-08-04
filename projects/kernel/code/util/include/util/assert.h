#ifndef UTIL_ASSERT_H
#define UTIL_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#if _MSC_VER
#define ASSERT(c)                                                              \
    if (!(c))                                                                  \
        __debugbreak();
#elif __GNUC__
#define ASSERT(c)                                                              \
    if (!(c)) {                                                                \
        /* __builtin_trap(); */                                                \
        *(volatile int *)0 = 0;                                                \
    }
#else
#define ASSERT(c)                                                              \
    if (!(c))                                                                  \
        *(volatile int *)0 = 0;
#endif
#else
#define ASSERT(c) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
