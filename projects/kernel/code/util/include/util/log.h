#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/array-types.h" // for char_a
#include "util/text/string.h" // for string
#include "util/types.h"       // for int64_t, uint64_t

#define NEWLINE 0x01
#define FLUSH 0x02

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// uint32 buffer
typedef struct {
    uint32_t *buffer;
    uint64_t size;
    uint32_t width;
    uint32_t height;
    uint32_t scanline;
} ScreenDimension;
void setupScreen(ScreenDimension dimension);
// TODO: needs buffer as argument when memory is set up
void appendToFlushBuffer(string data, unsigned char flags);
void flushBuffer(uint8_max_a *buffer);

void printToSerial(string data, uint8_t flags);

string stringWithMinSize(string data, unsigned char minSize, char_a tmp);
string stringWithMinSizeDefault(string data, unsigned char minSize);

string boolToString(bool data);

string ptrToString(void *data, char_a tmp);
string ptrToStringDefault(void *data);

string charToString(char data, char_a tmp);
string charToStringDefault(char data);

string stringToString(string data);

string uint64ToString(uint64_t data, char_a tmp);
string uint64ToStringDefault(uint64_t data);

string int64ToString(int64_t data, char_a tmp);
string int64ToStringDefault(int64_t data);

string doubleToString(double data, char_a tmp);
string doubleToStringDefault(double data);

string noAppend();

#define CONVERT_TO_STRING(data)                                                \
    _Generic((data),                                                           \
        string: stringToString,                                                \
        void *: ptrToStringDefault,                                            \
        int *: ptrToStringDefault,                                             \
        uint8_t *: ptrToStringDefault,                                         \
        unsigned int *: ptrToStringDefault,                                    \
        char: charToStringDefault,                                             \
        int64_t: int64ToStringDefault,                                         \
        double: doubleToStringDefault,                                         \
        uint64_t: uint64ToStringDefault,                                       \
        uint32_t: uint64ToStringDefault,                                       \
        uint16_t: uint64ToStringDefault,                                       \
        uint8_t: uint64ToStringDefault,                                        \
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

#ifdef __cplusplus
}
#endif

#endif
