#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"            // for U32, U8, U64, I64, I8, U16
#include "util/array-types.h" // for u_I8_a, uint8_max_a
#include "util/macros.h"      // for MACRO_VAR
#include "util/text/string.h" // for string

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
bool flushBuffer(uint8_max_a *buffer);

void rewind(U16 numberOfScreenLines);
void prowind(U16 numberOfScreenLines);

void printToSerial(string data, U8 flags);

string stringWithMinSize(string data, U8 minSize, u_I8_a tmp);
string stringWithMinSizeDefault(string data, U8 minSize);

string boolToString(bool data);

string ptrToString(void *data, u_I8_a tmp);
string ptrToStringDefault(void *data);

string I8ToString(I8 data, u_I8_a tmp);
string I8ToStringDefault(I8 data);

string stringToString(string data);

string uint64ToString(U64 data, u_I8_a tmp);
string uint64ToStringDefault(U64 data);

string int64ToString(I64 data, u_I8_a tmp);
string int64ToStringDefault(I64 data);

string doubleToString(double data, u_I8_a tmp);
string doubleToStringDefault(double data);

string noAppend();

#define CONVERT_TO_STRING(data)                                                \
    _Generic((data),                                                           \
        string: stringToString,                                                \
        void *: ptrToStringDefault,                                            \
        int *: ptrToStringDefault,                                             \
        U8 *: ptrToStringDefault,                                              \
        unsigned int *: ptrToStringDefault,                                    \
        I8: I8ToStringDefault,                                                 \
        I64: int64ToStringDefault,                                             \
        double: doubleToStringDefault,                                         \
        U64: uint64ToStringDefault,                                            \
        U32: uint64ToStringDefault,                                            \
        U16: uint64ToStringDefault,                                            \
        U8: uint64ToStringDefault,                                             \
        int: int64ToStringDefault,                                             \
        short: int64ToStringDefault,                                           \
        bool: boolToString,                                                    \
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
