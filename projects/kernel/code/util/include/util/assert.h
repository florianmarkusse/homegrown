#ifndef UTIL_ASSERT_H
#define UTIL_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#if _MSC_VER
#define FLO_ASSERT(c)                                                          \
    if (!(c))                                                                  \
        __debugbreak();
#elif __GNUC__
#define FLO_ASSERT_1(c)                                                        \
    if (!(c)) {                                                                \
        __builtin_trap();                                                      \
        *(volatile int *)0 = 0;                                                \
    }
#define FLO_ASSERT_2(c, text)                                                  \
    if (!(c)) {                                                                \
        *(volatile int *)0 = 0;                                                \
    }

#define FLO_ASSERT_X(a, b, c, ...) c
#define FLO_ASSERT(...)                                                        \
    FLO_ASSERT_X(__VA_ARGS__, FLO_ASSERT_2, FLO_ASSERT_1)                      \
    (__VA_ARGS__)

#else
#define FLO_ASSERT(c)                                                          \
    if (!(c))                                                                  \
        *(volatile int *)0 = 0;
#endif
#else
#define FLO_ASSERT(c) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
