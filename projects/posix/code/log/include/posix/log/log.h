#ifndef POSIX_LOG_LOG_H
#define POSIX_LOG_LOG_H

#include "interoperation/array-types.h"
#include "interoperation/macros.h"
#include "interoperation/types.h"
#include "shared/log.h"
#include "shared/text/converter.h"
#include "shared/text/string.h"

typedef struct {
    U8_max_a array;
    int fileDescriptor;
} WriteBuffer;

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

bool appendToFlushBuffer(string data, WriteBuffer *buffer, U8 flags);
bool flushBuffer(WriteBuffer *buffer);

bool appendColor(AnsiColor color, BufferType bufferType);
bool appendColorReset(BufferType bufferType);

WriteBuffer *getWriteBuffer(BufferType bufferType);

#define LOG_3(data, bufferType, flags)                                         \
    LOG_DATA_BUFFER_TYPE(data, bufferType, flags)
#define LOG_2(data, bufferType) LOG_DATA_BUFFER_TYPE(data, bufferType, 0)
#define LOG_1(data) LOG_DATA_BUFFER_TYPE(data, STDOUT, 0)

#define LOG_CHOOSER_IMPL_3(arg1, arg2, arg3) LOG_3(arg1, arg2, arg3)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL(_1, _2, _3, N, ...) LOG_CHOOSER_IMPL_##N
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) LOG(data, STDOUT, ##__VA_ARGS__)

#define ERROR(data, ...) LOG(data, STDERR, ##__VA_ARGS__)

#define FLUSH_TO(bufferType) flushBuffer(getWriteBuffer(bufferType))

#define LOG_DATA_3(data, buffer, flags)                                        \
    appendToFlushBuffer(CONVERT_TO_STRING(data), buffer, flags)
#define LOG_DATA_2(data, buffer) LOG_DATA_3(data, buffer, 0)
#define LOG_DATA_X(a, b, c, d, ...) d
#define LOG_DATA(...)                                                          \
    LOG_DATA_X(__VA_ARGS__, LOG_DATA_3, LOG_DATA_2)                            \
    (__VA_ARGS__)

#define LOG_DATA_BUFFER_TYPE(data, bufferType, flags)                          \
    LOG_DATA_3(data, getWriteBuffer(bufferType), flags)

/*#define LOG_3(data, bufferType, flags) \*/
/*    LOG_DATA_BUFFER_TYPE(data, bufferType, flags)*/
/*#define LOG_2(data, bufferType) LOG_DATA_BUFFER_TYPE(data, bufferType, 0)*/
/*#define LOG_1(data) LOG_DATA_BUFFER_TYPE(data, STDOUT, 0)*/
/**/
/*#define LOG_CHOOSER_IMPL_3(arg1, arg2, arg3) LOG_3(arg1, arg2, arg3)*/
/*#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)*/
/*#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)*/
/*#define LOG_CHOOSER_IMPL(_1, _2, _3, N, ...) LOG_CHOOSER_IMPL_##N*/
/*#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)*/
/**/
/*#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)*/
/**/
/*#define INFO(data, ...) LOG(data, STDOUT, ##__VA_ARGS__)*/
/**/
/*#define ERROR(data, ...) LOG(data, STDERR, ##__VA_ARGS__)*/

#define FLUSH_AFTER(bufferType)                                                \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = (FLUSH_TO(bufferType), 1))

#endif
