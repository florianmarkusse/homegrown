#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h" // for U32, U8, U64, I64, I8, U16
#include "util/array-types.h"     // for U8_a, uint8_max_a
#include "util/macros.h"          // for MACRO_VAR
#include "util/text/string.h"     // for string

#define NEWLINE 0x01
#define FLUSH 0x02

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// uint32 buffer
typedef struct {
    U32 *screen;
    U32 *backingBuffer;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} ScreenDimension;
void setupScreen(ScreenDimension dimension);
// TODO: needs buffer as argument when memory is set up
void appendToFlushBuffer(string data, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

void rewind(U16 numberOfScreenLines);
void prowind(U16 numberOfScreenLines);

void printToSerial(string data, U8 flags);

string stringWithMinSize(string data, U8 minSize, U8_a tmp);
string stringWithMinSizeDefault(string data, U8 minSize);

string boolToString(bool data);

string ptrToString(void *data, U8_a tmp);
string ptrToStringDefault(void *data);

string stringToString(string data);

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

#define SERIAL_DATA(data, flags) printToSerial(CONVERT_TO_STRING(data), flags)

#define SERIAL_1(data) SERIAL_DATA(data, 0)
#define SERIAL_2(data, flags) SERIAL_DATA(data, flags)

#define SERIAL_CHOOSER_IMPL_1(arg1) SERIAL_1(arg1)
#define SERIAL_CHOOSER_IMPL_2(arg1, arg2) SERIAL_2(arg1, arg2)
#define SERIAL_CHOOSER(...) SERIAL_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define SERIAL_CHOOSER_IMPL(_1, _2, N, ...) SERIAL_CHOOSER_IMPL_##N

#define SERIAL(...) SERIAL_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define LOG_DATA(data, flags)                                                  \
    appendToFlushBuffer(CONVERT_TO_STRING(data), flags)

#define LOG_1(data) LOG_DATA(data, 0)
#define LOG_2(data, flags) LOG_DATA(data, flags)

#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define LOG_CHOOSER_IMPL(_1, _2, N, ...) LOG_CHOOSER_IMPL_##N

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define FLUSH_AFTER                                                            \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#ifdef __cplusplus
}
#endif

#endif
