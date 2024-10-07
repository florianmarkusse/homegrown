#ifndef UTIL_TEXT_STRING_H
#define UTIL_TEXT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <util/memory/arena.h>

#include "util/assert.h"

typedef struct {
    unsigned char *buf;
    ptrdiff_t len;
} flo_string;

static constexpr flo_string FLO_EMPTY_STRING = ((flo_string){NULL, 0});
#define FLO_STRING(s) ((flo_string){(unsigned char *)(s), sizeof(s) - 1})
#define FLO_STRING_LEN(s, len) ((flo_string){(unsigned char *)(s), len})
#define FLO_STRING_PTRS(begin, end)                                            \
    ((flo_string){(unsigned char *)(begin), ((end) - (begin))})

#define FLO_STRING_PRINT(string) (int)(string).len, (string).buf

#define FLO_STRING_APPEND(string1, string2, perm)                              \
    ({                                                                         \
        unsigned char *FLO_MACRO_VAR(appendingBuf) =                           \
            FLO_NEW(perm, unsigned char, (string1).len + (string2).len);       \
        memcpy(appendingBuf, (string1).buf, (string1).len);                    \
        memcpy(appendingBuf + (string1).len, (string2).buf, (string2).len);    \
        flo_string FLO_MACRO_VAR(appendedString) =                             \
            FLO_STRING_LEN(appendingBuf, (string1).len + (string2).len);       \
        FLO_MACRO_VAR(appendedString);                                         \
    })

__attribute__((unused)) static inline bool flo_stringEquals(flo_string a,
                                                            flo_string b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

__attribute__((unused)) static inline flo_string
flo_stringCopy(flo_string dest, flo_string src) {
    FLO_ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}
__attribute__((unused)) static inline unsigned char
flo_getChar(flo_string str, ptrdiff_t index) {
    FLO_ASSERT(index >= 0 && index < str.len);

    return str.buf[index];
}

__attribute__((unused)) static inline unsigned char
flo_getCharOr(flo_string str, ptrdiff_t index, char or) {
    if (index < 0 || index >= str.len) {
        return or ;
    }
    return str.buf[index];
}

__attribute__((unused)) static inline unsigned char *
flo_getCharPtr(flo_string str, ptrdiff_t index) {
    FLO_ASSERT(index >= 0 && index < str.len);

    return &str.buf[index];
}

__attribute__((unused)) static inline bool flo_containsChar(flo_string s,
                                                            unsigned char ch) {
    for (ptrdiff_t i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

__attribute__((unused)) static inline flo_string
flo_splitString(flo_string s, unsigned char token, ptrdiff_t from) {
    FLO_ASSERT(from >= 0 && from < s.len);

    for (ptrdiff_t i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (flo_string){.buf = flo_getCharPtr(s, from),
                                .len = i - from};
        }
    }

    return (flo_string){.buf = flo_getCharPtr(s, from), .len = s.len - from};
}

__attribute__((unused)) static inline ptrdiff_t
flo_firstOccurenceOfFrom(flo_string s, unsigned char ch, ptrdiff_t from) {
    FLO_ASSERT(from >= 0 && from < s.len);

    for (ptrdiff_t i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return i;
        }
    }
    return -1;
}
__attribute__((unused)) static inline ptrdiff_t
flo_firstOccurenceOf(flo_string s, unsigned char ch) {
    return flo_firstOccurenceOfFrom(s, ch, 0);
}

__attribute__((unused)) static inline ptrdiff_t
flo_lastOccurenceOf(flo_string s, unsigned char ch) {
    for (ptrdiff_t i = s.len - 1; i >= 0; i--) {
        if (s.buf[i] == ch) {
            return i;
        }
    }
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif
