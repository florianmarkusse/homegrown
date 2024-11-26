#ifndef TEXT_CONVERTER_H
#define TEXT_CONVERTER_H

#include "shared/types/array-types.h"
#include "shared/types/types.h"
#include "shared/text/string.h"

string stringWithMinSize(string data, U8 minSize, U8_a tmp);
string stringWithMinSizeDefault(string data, U8 minSize);

string stringToString(string data);

string charToString(char data, U8_a tmp);
string charToStringDefault(char data);

string boolToString(bool data);

string ptrToString(void *data, U8_a tmp);
string ptrToStringDefault(void *data);

string U64ToString(U64 data, U8_a tmp);
string U64ToStringDefault(U64 data);

string I64ToString(I64 data, U8_a tmp);
string I64ToStringDefault(I64 data);

string F64ToString(F64 data, U8_a tmp);
string F64ToStringDefault(F64 data);

string noAppend();

#define CONVERT_TO_STRING(data)                                                \
    _Generic((data),                                                           \
        string: stringToString,                                                \
        char: charToStringDefault,                                             \
        bool: boolToString,                                                    \
        void *: ptrToStringDefault,                                            \
        U8 *: ptrToStringDefault,                                              \
        I8 *: ptrToStringDefault,                                              \
        U16 *: ptrToStringDefault,                                             \
        I16 *: ptrToStringDefault,                                             \
        U32 *: ptrToStringDefault,                                             \
        I32 *: ptrToStringDefault,                                             \
        U64 *: ptrToStringDefault,                                             \
        I64 *: ptrToStringDefault,                                             \
        U8: U64ToStringDefault,                                                \
        I8: I64ToStringDefault,                                                \
        U16: U64ToStringDefault,                                               \
        I16: I64ToStringDefault,                                               \
        U32: U64ToStringDefault,                                               \
        I32: I64ToStringDefault,                                               \
        U64: U64ToStringDefault,                                               \
        I64: I64ToStringDefault,                                               \
        F64: F64ToStringDefault,                                               \
        default: noAppend)(data)

#endif
