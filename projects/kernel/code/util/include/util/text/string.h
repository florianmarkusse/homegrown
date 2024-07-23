#ifndef UTIL_TEXT_STRING_H
#define UTIL_TEXT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/memory/memory.h"
#include <util/memory/arena.h>

#include "util/assert.h"

#define EMPTY_STRING ((string){NULL, 0})
#define STRING(s) ((string){(U8 *)(s), sizeof(s) - 1})
#define STRING_LEN(s, len) ((string){(U8 *)(s), len})
#define STRING_PTRS(begin, end)                                                \
    ((string){(U8 *)(begin), ((end) - (begin))})

// #define STRING_PRINT(string) (int)(string).len, (string).buf

#define STRING_APPEND(string1, string2, perm)                                  \
    ({                                                                         \
        U8 *MACRO_VAR(appendingBuf) =                               \
            NEW(perm, U8, (string1).len + (string2).len);           \
        memcpy(appendingBuf, (string1).buf, (string1).len);                    \
        memcpy(appendingBuf + (string1).len, (string2).buf, (string2).len);    \
        string MACRO_VAR(appendedString) =                                     \
            STRING_LEN(appendingBuf, (string1).len + (string2).len);           \
        MACRO_VAR(appendedString);                                             \
    })

typedef struct {
    U8 *buf;
    U64 len;
} string;

__attribute__((unused)) static inline bool stringEquals(string a, string b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

__attribute__((unused)) static inline string stringCopy(string dest,
                                                        string src) {
    ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}
__attribute__((unused)) static inline U8 getChar(string str,
                                                            U64 index) {
    ASSERT(index < str.len);

    return str.buf[index];
}

__attribute__((unused)) static inline U8
getCharOr(string str, U64 index, char or) {
    if (index < 0 || index >= str.len) {
        return or ;
    }
    return str.buf[index];
}

__attribute__((unused)) static inline U8 *
getCharPtr(string str, U64 index) {
    ASSERT(index < str.len);

    return &str.buf[index];
}

__attribute__((unused)) static inline bool containsChar(string s,
                                                        U8 ch) {
    for (U64 i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

__attribute__((unused)) static inline string
splitString(string s, U8 token, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (string){.buf = getCharPtr(s, from), .len = i - from};
        }
    }

    return (string){.buf = getCharPtr(s, from), .len = s.len - from};
}

__attribute__((unused)) static inline I64
firstOccurenceOfFrom(string s, U8 ch, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return i;
        }
    }
    return -1;
}
__attribute__((unused)) static inline I64
firstOccurenceOf(string s, U8 ch) {
    return firstOccurenceOfFrom(s, ch, 0);
}

__attribute__((unused)) static inline I64
lastOccurenceOf(string s, U8 ch) {
    // Is uint here so it will wrap at 0
    for (U64 i = s.len - 1; i >= s.len; i--) {
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
