#ifndef EFI_STRING_H
#define EFI_STRING_H

#include "platform-abstraction/memory/manipulation.h"
#include "shared/types/types.h"

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

static inline U8 *AscigetPtr(AsciString str, U64 index) {
    return &str.buf[index];
}

static inline AsciString AsciSplitString(AsciString s, U8 token, U64 from) {
    for (U64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (AsciString){.buf = AscigetPtr(s, from), .len = i - from};
        }
    }

    return (AsciString){.buf = AscigetPtr(s, from), .len = s.len - from};
}

#define TOKENIZE_ASCI_STRING(_string, stringIter, token, startingPosition)     \
    for ((stringIter) =                                                        \
             (AsciStringIter){                                                 \
                 .string = AsciSplitString(_string, token, startingPosition),  \
                 .pos = (startingPosition)};                                   \
         (stringIter).pos < (_string).len;                                     \
         (stringIter).pos += (stringIter).string.len + 1,                      \
        (stringIter).string =                                                  \
             AsciSplitString(_string, token, (stringIter).pos))

#endif
