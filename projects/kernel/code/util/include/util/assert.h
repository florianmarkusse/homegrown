#ifndef UTIL_ASSERT_H
#define UTIL_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
// Use set $pc += 2 to resume exection
#define BREAKPOINT __asm__ __volatile__("1: jmp 1b");
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
#define BREAKPOINT ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif
