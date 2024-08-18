#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/macros.h"       // for MACRO_VAR
#include "util/memory/arena.h" // for arena
#include "util/text/string.h"  // for string
#include "util/types.h"        // for char_a, char_d_a
#include <stdbool.h>           // for false, true, bool
#include <stddef.h>            // for ptrdiff_t
#include <stdint.h>            // for uint32_t, uint64_t

#define NEWLINE 0x01
#define FLUSH 0x02

// TODO: is there a way to directly append to a write buffer instead of going
// through a string first?
typedef struct {
    char_d_a array;
    int fileDescriptor;
} WriteBuffer;

// When adding a value to this enum, also add the right ansi escape code in
// log.c
typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_RESET,
    COLOR_NUMS
} AnsiColor;

typedef enum { STDOUT, STDERR } BufferType;

#define FLUSH_TO(bufferType) flushBuffer(getWriteBuffer(bufferType))

uint32_t appendToSimpleBuffer(string data, char_d_a *array, Arena *perm);

bool flushBuffer(WriteBuffer *buffer);
WriteBuffer *getWriteBuffer(BufferType bufferType);
uint32_t appendToFlushBuffer(string data, WriteBuffer *buffer,
                             unsigned char flags);

uint32_t appendColor(AnsiColor color, BufferType bufferType);
uint32_t appendColorReset(BufferType bufferType);

string stringWithMinSize(string data, unsigned char minSize, char_a tmp);
string stringWithMinSizeDefault(string data, unsigned char minSize);

string boolToString(bool data);

string ptrToString(void *data, char_a tmp);
string ptrToStringDefault(void *data);

string cStrToString(char *data);

string charToString(char data, char_a tmp);
string charToStringDefault(char data);

string stringToString(string data);

string uint64ToString(uint64_t data, char_a tmp);
string uint64ToStringDefault(uint64_t data);

string ptrdiffToString(ptrdiff_t data, char_a tmp);
string ptrdiffToStringDefault(ptrdiff_t data);

string doubleToString(double data, char_a tmp);
string doubleToStringDefault(double data);

string noAppend();

#define CONVERT_TO_STRING(data)                                                \
    _Generic((data),                                                           \
        string: stringToString,                                                \
        char *: cStrToString,                                                  \
        unsigned char *: cStrToString,                                         \
        void *: ptrToStringDefault,                                            \
        int *: ptrToStringDefault,                                             \
        unsigned int *: ptrToStringDefault,                                    \
        char: charToStringDefault,                                             \
        ptrdiff_t: ptrdiffToStringDefault,                                     \
        double: doubleToStringDefault,                                         \
        uint64_t: uint64ToStringDefault,                                       \
        uint32_t: uint64ToStringDefault,                                       \
        uint16_t: uint64ToStringDefault,                                       \
        uint8_t: uint64ToStringDefault,                                        \
        int: ptrdiffToStringDefault,                                           \
        short: ptrdiffToStringDefault,                                         \
        bool: boolToString,                                                    \
        default: noAppend)(data)

#define APPEND_DATA(data, buffer, perm)                                        \
    appendToSimpleBuffer(CONVERT_TO_STRING(data), buffer, perm)

#define LOG_DATA_2(data, buffer) LOG_DATA_3(data, buffer, 0)
#define LOG_DATA_3(data, buffer, flags)                                        \
    appendToFlushBuffer(CONVERT_TO_STRING(data), buffer, flags)
#define LOG_DATA_X(a, b, c, d, ...) d
#define LOG_DATA(...)                                                          \
    LOG_DATA_X(__VA_ARGS__, LOG_DATA_3, LOG_DATA_2)                            \
    (__VA_ARGS__)

#define LOG_DATA_BUFFER_TYPE(data, bufferType, flags)                          \
    LOG_DATA_3(data, getWriteBuffer(bufferType), flags)

#define LOG_1(data) LOG_DATA_BUFFER_TYPE(data, STDOUT, 0)
#define LOG_2(data, bufferType) LOG_DATA_BUFFER_TYPE(data, bufferType, 0)
#define LOG_3(data, bufferType, flags)                                         \
    LOG_DATA_BUFFER_TYPE(data, bufferType, flags)

#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER_IMPL_3(arg1, arg2, arg3) LOG_3(arg1, arg2, arg3)
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)
#define LOG_CHOOSER_IMPL(_1, _2, _3, N, ...) LOG_CHOOSER_IMPL_##N

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) LOG(data, STDOUT, ##__VA_ARGS__)

#define ERROR(data, ...) LOG(data, STDERR, ##__VA_ARGS__)
#define APPEND_ERRNO                                                           \
    ERROR(STRING("Error code: "));                                             \
    ERROR(errno, NEWLINE);                                                     \
    ERROR(STRING("Error message: "));                                          \
    ERROR(strerror(errno), NEWLINE);
#define APPEND_ERRNO_RAW(value)                                                \
    ERROR(STRING("Error code: "));                                             \
    ERROR(value, NEWLINE);                                                     \
    ERROR(STRING("Error message: "));                                          \
    ERROR(strerror(value), NEWLINE);

#define FLUSH_AFTER(bufferType)                                                \
    for (ptrdiff_t MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                         \
         MACRO_VAR(i) = (FLUSH_TO(bufferType), 1))

#ifdef __cplusplus
}
#endif

#endif
