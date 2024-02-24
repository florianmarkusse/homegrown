#ifndef UTIL_TEXT_CHAR_H
#define UTIL_TEXT_CHAR_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((unused)) static inline unsigned char
flo_isAlphabetical(unsigned char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

__attribute__((unused)) static inline unsigned char
flo_isNumerical(unsigned char ch) {
    return (ch >= '0' && ch <= '9');
}

__attribute__((unused)) static inline unsigned char
flo_isFormattingCharacter(unsigned char ch) {
    return ch == '\t' || ch == '\n' || ch == '\r';
}

#ifdef __cplusplus
}
#endif

#endif
