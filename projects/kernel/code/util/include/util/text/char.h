#ifndef UTIL_TEXT_CHAR_H
#define UTIL_TEXT_CHAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

__attribute__((unused)) static inline U8 isAlphabetical(U8 ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

__attribute__((unused)) static inline U8 isNumerical(U8 ch) {
    return (ch >= '0' && ch <= '9');
}

__attribute__((unused)) static inline U8 isFormattingCharacter(U8 ch) {
    return ch == '\t' || ch == '\n' || ch == '\r';
}

#ifdef __cplusplus
}
#endif

#endif
