#include "kernel/memory/manipulation.h"
#include "shared/types/types.h"

#define NO_INLINE __attribute__((noinline))

#ifdef __clang__
typedef I8 U8_8 __attribute__((ext_vector_type(8), aligned(1)));
typedef I8 U8_16 __attribute__((ext_vector_type(16), aligned(1)));
typedef I8 U8_32 __attribute__((ext_vector_type(32), aligned(1)));
typedef I8 U8_32a __attribute__((ext_vector_type(32), aligned(32)));

#else
// __GNUC__
typedef I8 U8_8 __attribute__((vector_size(8), aligned(1)));
typedef I8 U8_16 __attribute__((vector_size(16), aligned(1)));
typedef I8 U8_32 __attribute__((vector_size(32), aligned(1)));
typedef I8 U8_32a __attribute__((vector_size(32), aligned(32)));
#endif

typedef U32 __attribute__((aligned(1))) u32;
typedef U64 __attribute__((aligned(1))) u64;

__attribute((nothrow, nonnull(1, 2))) void *
memcpy(void *__restrict dest, const void *__restrict src, U64 n) {
    I8 *d = (I8 *)dest;
    const I8 *s = (I8 *)src;

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
            const I8 *first_s = s;
            const I8 *last_s = s + n - 8;
            I8 *first_d = d;
            I8 *last_d = d + n - 8;
            *((u64 *)first_d) = *((u64 *)first_s);
            *((u64 *)last_d) = *((u64 *)last_s);
            return dest;
        }

        const I8 *first_s = s;
        const I8 *last_s = s + n - 4;
        I8 *first_d = d;
        I8 *last_d = d + n - 4;
        *((u32 *)first_d) = *((u32 *)first_s);
        *((u32 *)last_d) = *((u32 *)last_s);
        return dest;
    }

    if (n <= 32) {
        const I8 *first_s = s;
        const I8 *last_s = s + n - 16;
        I8 *first_d = d;
        I8 *last_d = d + n - 16;

        *((U8_16 *)first_d) = *((U8_16 *)first_s);
        *((U8_16 *)last_d) = *((U8_16 *)last_s);
        return dest;
    }

    const I8 *last_word_s = s + n - 32;
    I8 *last_word_d = d + n - 32;

    // Stamp the 32-byte chunks.
    do {
        *((U8_32 *)d) = *((U8_32 *)s);
        d += 32;
        s += 32;
    } while (d < last_word_d);

    // Stamp the last unaligned 32 bytes of the buffer.
    *((U8_32 *)last_word_d) = *((U8_32 *)last_word_s);
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
                                                    U64 n) {
    I8 *d = dest;
    const I8 *s = src;

    if (d == s) {
        return d;
    }
    if (((U64)s - (U64)d - n) <= -2ULL * n) {
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
        I8 *p = s;
        p[0] = (I8)c;
        p[n - 1] = (I8)c;
        if (n <= 2)
            return s;
        p[1] = (I8)c;
        p[2] = (I8)c;
        return s;
    }

    if (n <= 16) {
        U64 val8 = ((U64)0x0101010101010101L * ((U8)c));
        if (n >= 8) {
            I8 *first = s;
            I8 *last = s + n - 8;
            *((u64 *)first) = val8;
            *((u64 *)last) = val8;
            return s;
        }

        U32 val4 = (U32)val8;
        I8 *first = s;
        I8 *last = s + n - 4;
        *((u32 *)first) = val4;
        *((u32 *)last) = val4;
        return s;
    }

    I8 X = (I8)c;
    I8 *p = s;
    U8_16 val16 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};
    I8 *last = s + n - 16;
    *((U8_16 *)last) = val16;
    *((U8_16 *)p) = val16;
    return s;
}

static inline void *huge_memset(void *s, int c, U64 n) {
    I8 *p = s;
    I8 X = (I8)c;
    U8_32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                   X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    // Stamp the first 32byte store.
    *((U8_32 *)p) = val32;

    I8 *first_aligned = p + 32 - ((U64)p % 32);
    I8 *buffer_end = p + n;
    I8 *last_word = buffer_end - 32;

    // Align the next stores.
    p = first_aligned;

    // Unroll the body of the loop to increase parallelism.
    while (p + (32 * 5) < buffer_end) {
        *((U8_32a *)p) = val32;
        p += 32;
        *((U8_32a *)p) = val32;
        p += 32;
        *((U8_32a *)p) = val32;
        p += 32;
        *((U8_32a *)p) = val32;
        p += 32;
        *((U8_32a *)p) = val32;
        p += 32;
    }

// Complete the last few iterations:
#define TRY_STAMP_32_BYTES                                                     \
    if (p < last_word) {                                                       \
        *((U8_32a *)p) = val32;                                                \
        p += 32;                                                               \
    }

    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES
    TRY_STAMP_32_BYTES

    // Stamp the last unaligned word.
    *((U8_32 *)last_word) = val32;
    return s;
}

__attribute((nothrow, nonnull(1))) void *memset(void *s, int c, U64 n) {
    I8 *p = s;
    I8 X = (I8)c;

    if (n < 32) {
        return small_memset(s, c, n);
    }

    if (n > 160) {
        return huge_memset(s, c, n);
    }

    U8_32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                   X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

    I8 *last_word = s + n - 32;

    // Stamp the 32-byte chunks.
    do {
        *((U8_32 *)p) = val32;
        p += 32;
    } while (p < last_word);

    // Stamp the last unaligned 32 bytes of the buffer.
    *((U8_32 *)last_word) = val32;
    return s;
}

/* Compare N bytes of S1 and S2.  */
__attribute((nothrow, pure, nonnull(1, 2))) int memcmp(const void *s1,
                                                       const void *s2, U64 n) {
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
