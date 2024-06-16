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
#define ASSERT_1(c)                                                            \
    if (!(c)) {                                                                \
        __asm__ __volatile__("int $3");                                        \
    }
#define ASSERT_2(c, text)                                                      \
    if (!(c)) {                                                                \
        __asm__ __volatile__("int $3");                                        \
    }

#define ASSERT_X(a, b, c, ...) c
#define ASSERT(...)                                                            \
    ASSERT_X(__VA_ARGS__, ASSERT_2, ASSERT_1)                                  \
    (__VA_ARGS__)

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
