#include "memory/standard.h"

#define NO_INLINE __attribute__((noinline))

#ifdef __clang__
typedef char char8 __attribute__((ext_vector_type(8), aligned(1)));
typedef char char16 __attribute__((ext_vector_type(16), aligned(1)));
typedef char char32 __attribute__((ext_vector_type(32), aligned(1)));
typedef char char32a __attribute__((ext_vector_type(32), aligned(32)));

#else
// __GNUC__
typedef char char8 __attribute__((vector_size(8), aligned(1)));
typedef char char16 __attribute__((vector_size(16), aligned(1)));
typedef char char32 __attribute__((vector_size(32), aligned(1)));
typedef char char32a __attribute__((vector_size(32), aligned(32)));
#endif

typedef U32 __attribute__((aligned(1))) u32;
typedef U64 __attribute__((aligned(1))) u64;

__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, I64 n) {
    char *d = (char *)dest;
    const char *s = (char *)src;

    if (n < 5) {
        if (n == 0)
            return dest;
        d[0] = s[0];
        d[n - 1] = s[n - 1];
        if (n <= 2)
            return dest;
        d[1] = s[1];
        d[2] = s[2];
        return dest;
    }

    if (n <= 16) {
        if (n >= 8) {
            const char *first_s = s;
            const char *last_s = s + n - 8;
            char *first_d = d;
            char *last_d = d + n - 8;
            *((u64 *)first_d) = *((u64 *)first_s);
            *((u64 *)last_d) = *((u64 *)last_s);
            return dest;
        }

        const char *first_s = s;
        const char *last_s = s + n - 4;
        char *first_d = d;
        char *last_d = d + n - 4;
        *((u32 *)first_d) = *((u32 *)first_s);
        *((u32 *)last_d) = *((u32 *)last_s);
        return dest;
    }

    if (n <= 32) {
        const char *first_s = s;
        const char *last_s = s + n - 16;
        char *first_d = d;
        char *last_d = d + n - 16;

        *((char16 *)first_d) = *((char16 *)first_s);
        *((char16 *)last_d) = *((char16 *)last_s);
        return dest;
    }

    const char *last_word_s = s + n - 32;
    char *last_word_d = d + n - 32;

    // Stamp the 32-byte chunks.
    do {
        *((char32 *)d) = *((char32 *)s);
        d += 32;
        s += 32;
    } while (d < last_word_d);

    // Stamp the last unaligned 32 bytes of the buffer.
    *((char32 *)last_word_d) = *((char32 *)last_word_s);
    return dest;
}

#ifdef __GNUC__
typedef __attribute__((__may_alias__)) U64 WT;
#define WS (sizeof(WT))
#endif

/*
 * https://stackoverflow.com/questions/57639853/is-comparing-two-pointers-with-undefined-behavior-if-they-are-both-cast-to-an
 *
 * http://git.musl-libc.org/cgit/musl/tree/src/string/memmove.c
 *
 * Dont use UB unless people have been doing it for decades pepelaugh.
 */
__attribute((nothrow, nonnull(1, 2))) void *memmove(void *dest, const void *src,
                                                    I64 n) {
    char *d = dest;
    const char *s = src;

    if (d == s) {
        return d;
    }
    if ((I64)((U64)s - (U64)d - n) <= -2 * n) {
        return memcpy(d, s, n);
    }

    if (d < s) {
#ifdef __GNUC__
        if ((U64)s % WS == (U64)d % WS) {
            while ((U64)d % WS) {
                if (!n--) {
                    return dest;
                }
                *d++ = *s++;
            }
            for (; (U64)n >= WS; n -= WS, d += WS, s += WS) {
                *(WT *)d = *(WT *)s;
            }
        }
#endif
        for (; n; n--) {
            *d++ = *s++;
        }
    } else {
#ifdef __GNUC__
        if ((U64)s % WS == (U64)d % WS) {
            while ((U64)(d + n) % WS) {
                if (!n--) {
                    return dest;
                }
                d[n] = s[n];
            }
            while ((U64)n >= WS) {
                n -= WS, *(WT *)(d + n) = *(WT *)(s + n);
            }
        }
#endif
        while (n) {
            n--, d[n] = s[n];
        }
    }

    return dest;
}

// Handle memsets of sizes 0..32
static inline void *small_memset(void *s, int c, U64 n) {
    if (n < 5) {
        if (n == 0)
            return s;
        char *p = s;
        p[0] = (char)c;
        p[n - 1] = (char)c;
        if (n <= 2)
            return s;
        p[1] = (char)c;
        p[2] = (char)c;
        return s;
    }

    if (n <= 16) {
        U64 val8 = ((U64)0x0101010101010101L * ((U8)c));
        if (n >= 8) {
            char *first = s;
            char *last = s + n - 8;
            *((u64 *)first) = val8;
            *((u64 *)last) = val8;
            return s;
        }

        U32 val4 = (U32)val8;
        char *first = s;
        char *last = s + n - 4;
        *((u32 *)first) = val4;
        *((u32 *)last) = val4;
        return s;
    }

    char X = (char)c;
    char *p = s;
    char16 val16 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};
    char *last = s + n - 16;
    *((char16 *)last) = val16;
    *((char16 *)p) = val16;
    return s;
}

static inline void *huge_memset(void *s, int c, U64 n) {
    char *p = s;
    char X = (char)c;
    char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    // Stamp the first 32byte store.
    *((char32 *)p) = val32;

    char *first_aligned = p + 32 - ((U64)p % 32);
    char *buffer_end = p + n;
    char *last_word = buffer_end - 32;

    // Align the next stores.
    p = first_aligned;

    // Unroll the body of the loop to increase parallelism.
    while (p + (32 * 5) < buffer_end) {
        *((char32a *)p) = val32;
        p += 32;
        *((char32a *)p) = val32;
        p += 32;
        *((char32a *)p) = val32;
        p += 32;
        *((char32a *)p) = val32;
        p += 32;
        *((char32a *)p) = val32;
        p += 32;
    }

// Complete the last few iterations:
#define TRY_STAMP_32_BYTES                                                     \
    if (p < last_word) {                                                       \
        *((char32a *)p) = val32;                                               \
        p += 32;                                                               \
    }

    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES

    // Stamp the last unaligned word.
    *((char32 *)last_word) = val32;
    return s;
}

__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, I64 n) {
    char *p = s;
    char X = (char)c;

    if (n < 32) {
        return small_memset(s, c, n);
    }

    if (n > 160) {
        return huge_memset(s, c, n);
    }

    char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                    X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    char *last_word = s + n - 32;

    // Stamp the 32-byte chunks.
    do {
        *((char32 *)p) = val32;
        p += 32;
    } while (p < last_word);

    // Stamp the last unaligned 32 bytes of the buffer.
    *((char32 *)last_word) = val32;
    return s;
}

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int memcmp(const void *s1,
                                                       const void *s2, I64 n) {
    U64 i;

    /**
     * p1 and p2 are the same memory? easy peasy! bail out
     */
    if (s1 == s2) {
        return 0;
    }

    // This for loop does the comparing and pointer moving...
    for (i = 0; (i < (U64)n) && (*(U8 *)s1 == *(U8 *)s2);
         i++, s1 = 1 + (U8 *)s1, s2 = 1 + (U8 *)s2)
        ;

    // if i == length, then we have passed the test
    return (i == (U64)n) ? 0 : (*(U8 *)s1 - *(U8 *)s2);
}
