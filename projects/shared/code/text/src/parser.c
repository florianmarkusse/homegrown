#include "shared/text/parser.h"

U32 parseU32(string value, U8 base) {
    U32 result = 0;
    for (U64 i = 0; i < value.len; i++) {
        U8 digit = value.buf[i] - '0';
        result = (result * base) + digit;
    }
    return result;
}

U16 parseU16(string value, U8 base) {
    U16 result = 0;
    for (U64 i = 0; i < value.len; i++) {
        U8 digit = value.buf[i] - '0';
        result = (result * base) + digit;
    }
    return result;
}
