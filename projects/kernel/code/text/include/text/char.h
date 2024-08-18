#ifndef UTIL_TEXT_CHAR_H
#define UTIL_TEXT_CHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

static inline U8 isAlphabetical(U8 ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static inline U8 isNumerical(U8 ch) { return (ch >= '0' && ch <= '9'); }

static inline U8 isFormattingCharacter(U8 ch) {
    return ch == '\t' || ch == '\n' || ch == '\r';
}

#ifdef __cplusplus
}
#endif

#endif