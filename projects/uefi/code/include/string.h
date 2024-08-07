#ifndef STRING_H
#define STRING_H

#include "efi/c-efi-base.h"
#include "memory/standard.h"

typedef struct {
    U8 *buf;
    U64 len;
} AsciString;

#define ASCI_STRING(s) ((AsciString){(U8 *)(s), ((sizeof(s) - 1))})
#define ASCI_STRING_LEN(s, n)                                                  \
    (AsciString) { (U8 *)(s), n }

typedef struct {
    AsciString string;
    U64 pos;
} AsciStringIter;

static inline bool asciStringEquals(AsciString a, AsciString b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

#define TOKENIZE_ASCI_STRING(_string, stringIter, token, startingPosition)     \
    for ((stringIter) =                                                        \
             (AsciStringIter){                                                 \
                 .string = splitString(_string, token, startingPosition),      \
                 .pos = (startingPosition)};                                   \
         (stringIter).pos < (_string).len;                                     \
         (stringIter).pos += (stringIter).string.len + 1,                      \
        (stringIter).string = splitString(_string, token, (stringIter).pos))

static inline U8 *getPtr(AsciString str, U64 index) {
    return &str.buf[index];
}

static inline AsciString splitString(AsciString s, U8 token,
                                     U64 from) {
    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (AsciString){.buf = getPtr(s, from), .len = i - from};
        }
    }

    return (AsciString){.buf = getPtr(s, from), .len = s.len - from};
}

#endif
