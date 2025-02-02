#ifndef SHARED_HASH_HASHES_H
#define SHARED_HASH_HASHES_H

#include "shared/types/types.h"

#include "shared/text/string.h"

static inline U64 hashStringSkeeto(string string) {
    U64 h = 0x100;
    for (U64 i = 0; i < string.len; i++) {
        h ^= string.buf[i];
        h *= 1111111111111111111u;
    }
    return h;
}

// https://github.com/skeeto/hash-prospector
static inline U32 hashU32(U32 x) {
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0x735a2d97;
    x ^= x >> 15;
    return x;
}

// https://github.com/skeeto/hash-prospector
// 3-round xorshift-multiply (-Xn3)
// bias = 0.0045976709018820602
static inline U16 hashU16(U16 x) {
    x ^= x >> 7;
    x *= 0x2993U;
    x ^= x >> 5;
    x *= 0xe877U;
    x ^= x >> 9;
    x *= 0x0235U;
    x ^= x >> 10;
    return x;
}

#endif
