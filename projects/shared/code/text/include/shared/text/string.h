#ifndef UTIL_TEXT_STRING_H
#define UTIL_TEXT_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/array.h"
#include "interoperation/types.h"
#include "platform-abstraction/assert.h"
#include "shared/memory/manipulation/manipulation.h"

typedef struct {
    U8 *buf;
    U64 len;
} string;

typedef DYNAMIC_ARRAY(string) string_d_a;
typedef MAX_LENGTH_ARRAY(string) string_max_a;

static constexpr string EMPTY_STRING = ((string){0, 0});
#define STRING(s) ((string){(U8 *)(s), sizeof(s) - 1})
#define STRING_LEN(s, len) ((string){(U8 *)(s), len})
#define STRING_PTRS(begin, end) ((string){(U8 *)(begin), ((end) - (begin))})

#define STRING_APPEND(string1, string2, perm)                                  \
    ({                                                                         \
        U8 *MACRO_VAR(appendingBuf) =                                          \
            NEW(perm, U8, (string1).len + (string2).len);                      \
        memcpy(appendingBuf, (string1).buf, (string1).len);                    \
        memcpy(appendingBuf + (string1).len, (string2).buf, (string2).len);    \
        string MACRO_VAR(appendedString) =                                     \
            STRING_LEN(appendingBuf, (string1).len + (string2).len);           \
        MACRO_VAR(appendedString);                                             \
    })

static inline bool stringEquals(string a, string b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

static inline string stringCopy(string dest, string src) {
    ASSERT(dest.len >= src.len);

    memcpy(dest.buf, src.buf, src.len);
    dest.len = src.len;
    return dest;
}
static inline U8 getChar(string str, U64 index) {
    ASSERT(index < str.len);

    return str.buf[index];
}

static inline U8 getCharOr(string str, U64 index, I8 or) {
    if (index < 0 || index >= str.len) {
        return or ;
    }
    return str.buf[index];
}

static inline U8 *getCharPtr(string str, U64 index) {
    ASSERT(index < str.len);

    return &str.buf[index];
}

static inline bool containsChar(string s, U8 ch) {
    for (U64 i = 0; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return true;
        }
    }
    return false;
}

static inline string splitString(string s, U8 token, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (string){.buf = getCharPtr(s, from), .len = i - from};
        }
    }

    return (string){.buf = getCharPtr(s, from), .len = s.len - from};
}

static inline I64 firstOccurenceOfFrom(string s, U8 ch, U64 from) {
    ASSERT(from >= 0 && from < s.len);

    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == ch) {
            return i;
        }
    }
    return -1;
}
static inline I64 firstOccurenceOf(string s, U8 ch) {
    return firstOccurenceOfFrom(s, ch, 0);
}

static inline I64 lastOccurenceOf(string s, U8 ch) {
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
