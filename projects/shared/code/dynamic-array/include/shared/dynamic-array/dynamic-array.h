#include "shared/macros.h"
#include "shared/types/types.h"
#include "shared/memory/allocator/arena.h"

void grow(void *slice, U64 size, U64 align, Arena *a, U8 flags);

#define PUSH_2(s, a)                                                           \
    ({                                                                         \
        typeof(s) MACRO_VAR(s_) = (s);                                         \
        typeof(a) MACRO_VAR(a_) = (a);                                         \
        if (MACRO_VAR(s_)->len >= MACRO_VAR(s_)->cap) {                        \
            grow(MACRO_VAR(s_), sizeof(*MACRO_VAR(s_)->buf),                   \
                 alignof(*MACRO_VAR(s_)->buf), MACRO_VAR(a_), 0);              \
        }                                                                      \
        MACRO_VAR(s_)->buf + MACRO_VAR(s_)->len++;                             \
    })
#define PUSH_3(s, a, f)                                                        \
    ({                                                                         \
        typeof(s) MACRO_VAR(s_) = (s);                                         \
        typeof(a) MACRO_VAR(a_) = (a);                                         \
        if (MACRO_VAR(s_)->len >= MACRO_VAR(s_)->cap) {                        \
            grow(MACRO_VAR(s_), sizeof(*MACRO_VAR(s_)->buf),                   \
                 alignof(*MACRO_VAR(s_)->buf), MACRO_VAR(a_), f);              \
        }                                                                      \
        MACRO_VAR(s_)->buf + MACRO_VAR(s_)->len++;                             \
    })
#define PUSH_X(a, b, c, d, ...) d
#define PUSH(...) PUSH_X(__VA_ARGS__, PUSH_3, PUSH_2)(__VA_ARGS__)
