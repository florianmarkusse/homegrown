#ifndef INTEROPERATION_ASSERT_H
#define INTEROPERATION_ASSERT_H

#ifdef DEBUG

#ifdef FREESTANDING_BUILD
// Use set $pc += 2 to resume exection
#define BREAKPOINT __asm__ __volatile__("1: jmp 1b");
#define ASSERT(c)                                                              \
    if (!(c)) {                                                                \
        __asm__ __volatile__("1: jmp 1b");                                     \
    }
#else

#define BREAKPOINT __asm__ __volatile__("int3; nop");

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

#endif

#else
#define ASSERT(c) ((void)0)
#define BREAKPOINT ((void)0)
#endif

#endif
