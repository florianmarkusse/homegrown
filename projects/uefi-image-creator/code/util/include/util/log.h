#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/allocator/arena.h" // for Arena
#include "types.h"                  // for flo_U8_a, flo_U8_d_a
#include "util/macros.h"            // for FLO_MACRO_VAR
#include "util/text/string.h"       // for string, STRING
#include <stddef.h>                 // for U64
#include <stdint.h>                 // for U32, U64

static constexpr auto FLO_NEWLINE = 0x01;
static constexpr auto FLO_FLUSH = 0x02;

// TODO: is there a way to directly append to a write buffer instead of going
// through a string first?
typedef struct {
    flo_U8_d_a array;
    int fileDescriptor;
} flo_WriteBuffer;

// When adding a value to this enum, also add the right ansi escape code in
// log.c
typedef enum {
    FLO_COLOR_RED,
    FLO_COLOR_GREEN,
    FLO_COLOR_YELLOW,
    FLO_COLOR_BLUE,
    FLO_COLOR_MAGENTA,
    FLO_COLOR_CYAN,
    FLO_COLOR_RESET,
    FLO_COLOR_NUMS
} flo_AnsiColor;

typedef enum { FLO_STDOUT, FLO_STDERR } flo_BufferType;

#define FLO_FLUSH_TO(bufferType) flo_flushBuffer(flo_getWriteBuffer(bufferType))

U32 flo_appendToSimpleBuffer(string data, flo_U8_d_a *array, Arena *perm);

bool flo_flushBuffer(flo_WriteBuffer *buffer);
flo_WriteBuffer *flo_getWriteBuffer(flo_BufferType bufferType);
U32 flo_appendToFlushBuffer(string data, flo_WriteBuffer *buffer, U8 flags);

U32 flo_appendColor(flo_AnsiColor color, flo_BufferType bufferType);
U32 flo_appendColorReset(flo_BufferType bufferType);

string stringWithMinSize(string data, U8 minSize, flo_U8_a tmp);
string stringWithMinSizeDefault(string data, U8 minSize);

string flo_boolToString(bool data);

string flo_ptrToString(void *data, flo_U8_a tmp);
string flo_ptrToStringDefault(void *data);

string flo_cStrToString(U8 *data);

string flo_U8ToString(U8 data, flo_U8_a tmp);
string flo_U8ToStringDefault(U8 data);

string stringToString(string data);

string flo_uint64ToString(U64 data, flo_U8_a tmp);
string flo_uint64ToStringDefault(U64 data);

string flo_ptrdiffToString(U64 data, flo_U8_a tmp);
string flo_ptrdiffToStringDefault(U64 data);

string flo_doubleToString(double data, flo_U8_a tmp);
string flo_doubleToStringDefault(double data);

string flo_noAppend();

#define FLO_CONVERT_TO_STRING(data)                                            \
    _Generic((data),                                                           \
        string: stringToString,                                                \
        unsigned char *: flo_cStrToString,                                     \
        U8 *: flo_cStrToString,                                                \
        void *: flo_ptrToStringDefault,                                        \
        int *: flo_ptrToStringDefault,                                         \
        U8 **: flo_ptrToStringDefault,                                         \
        unsigned int *: flo_ptrToStringDefault,                                \
        U8: flo_U8ToStringDefault,                                             \
        U64: flo_ptrdiffToStringDefault,                                       \
        double: flo_doubleToStringDefault,                                     \
        U64: flo_uint64ToStringDefault,                                        \
        U32: flo_uint64ToStringDefault,                                        \
        U16: flo_uint64ToStringDefault,                                        \
        uint8_t: flo_uint64ToStringDefault,                                    \
        int: flo_ptrdiffToStringDefault,                                       \
        short: flo_ptrdiffToStringDefault,                                     \
        bool: flo_boolToString,                                                \
        default: flo_noAppend)(data)

#define FLO_APPEND_DATA(data, buffer, perm)                                    \
    flo_appendToSimpleBuffer(FLO_CONVERT_TO_STRING(data), buffer, perm)

#define FLO_LOG_DATA_2(data, buffer) FLO_LOG_DATA_3(data, buffer, 0)
#define FLO_LOG_DATA_3(data, buffer, flags)                                    \
    flo_appendToFlushBuffer(FLO_CONVERT_TO_STRING(data), buffer, flags)
#define FLO_LOG_DATA_X(a, b, c, d, ...) d
#define FLO_LOG_DATA(...)                                                      \
    FLO_LOG_DATA_X(__VA_ARGS__, FLO_LOG_DATA_3, FLO_LOG_DATA_2)                \
    (__VA_ARGS__)

#define FLO_LOG_DATA_BUFFER_TYPE(data, bufferType, flags)                      \
    FLO_LOG_DATA_3(data, flo_getWriteBuffer(bufferType), flags)

#define FLO_LOG_1(data) FLO_LOG_DATA_BUFFER_TYPE(data, FLO_STDOUT, 0)
#define FLO_LOG_2(data, bufferType)                                            \
    FLO_LOG_DATA_BUFFER_TYPE(data, bufferType, 0)
#define FLO_LOG_3(data, bufferType, flags)                                     \
    FLO_LOG_DATA_BUFFER_TYPE(data, bufferType, flags)

#define FLO_LOG_CHOOSER_IMPL_1(arg1) FLO_LOG_1(arg1)
#define FLO_LOG_CHOOSER_IMPL_2(arg1, arg2) FLO_LOG_2(arg1, arg2)
#define FLO_LOG_CHOOSER_IMPL_3(arg1, arg2, arg3) FLO_LOG_3(arg1, arg2, arg3)
#define FLO_LOG_CHOOSER(...) FLO_LOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)
#define FLO_LOG_CHOOSER_IMPL(_1, _2, _3, N, ...) FLO_LOG_CHOOSER_IMPL_##N

#define FLO_LOG(...) FLO_LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define FLO_INFO(data, ...) FLO_LOG(data, FLO_STDOUT, ##__VA_ARGS__)

#define FLO_ERROR(data, ...) FLO_LOG(data, FLO_STDERR, ##__VA_ARGS__)
#define FLO_APPEND_ERRNO_RAW(value)                                            \
    FLO_ERROR(STRING("Error code: "));                                         \
    FLO_ERROR(value, FLO_NEWLINE);                                             \
    FLO_ERROR(STRING("Error message: "));                                      \
    FLO_ERROR(strerror(value), FLO_NEWLINE);

#define FLO_FLUSH_AFTER(bufferType)                                            \
    for (U64 FLO_MACRO_VAR(i) = 0; FLO_MACRO_VAR(i) < 1;                       \
         FLO_MACRO_VAR(i) = (FLO_FLUSH_TO(bufferType), 1))

#ifdef __cplusplus
}
#endif

#endif
