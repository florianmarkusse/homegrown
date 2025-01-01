#ifndef POSIX_LOG_H
#define POSIX_LOG_H

#include "shared/macros.h"
#include "shared/text/converter.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"

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

bool appendToFlushBufferWithWriter(string data, U8 flags, WriteBuffer *buffer);
bool appendZeroToFlushBufferWithWriter(U64 bytes, U8 flags,
                                       WriteBuffer *buffer);
bool flushBufferWithWriter(WriteBuffer *buffer);
bool flushBufferWithFileDescriptor(int fileDescriptor, U8 *buffer, U64 size);

bool appendColor(AnsiColor color, BufferType bufferType);
bool appendColorReset(BufferType bufferType);

WriteBuffer *getWriteBuffer(BufferType bufferType);

#define PLOG_DATA(data, flags, buffer)                                         \
    appendToFlushBufferWithWriter(CONVERT_TO_STRING(data), flags, buffer)

#define PLOG_DATA_BUFFER_TYPE(data, flags, bufferType)                         \
    PLOG_DATA(data, flags, getWriteBuffer(bufferType))

#define PLOG_3(data, flags, bufferType)                                        \
    PLOG_DATA_BUFFER_TYPE(data, flags, bufferType)
#define PLOG_2(data, flags) PLOG_DATA_BUFFER_TYPE(data, flags, STDOUT)
#define PLOG_1(data) PLOG_DATA_BUFFER_TYPE(data, 0, STDOUT)
#define PLOG_CHOOSER_IMPL(_1, _2, _3, N, ...) PLOG_##N
#define PLOG_CHOOSER(...) PLOG_CHOOSER_IMPL(__VA_ARGS__, 3, 2, 1)

#define PLOG(...) PLOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define PINFO_2(data, flags) PLOG(data, flags, STDOUT)
#define PINFO_1(data) PLOG(data, 0, STDOUT)
#define PINFO_CHOOSER_IMPL(_1, _2, N, ...) PINFO_##N
#define PINFO_CHOOSER(...) PINFO_CHOOSER_IMPL(__VA_ARGS__, 2, 1)

#define PINFO(...) PINFO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define PERROR_2(data, flags) PLOG(data, flags, STDERR)
#define PERROR_1(data) PLOG(data, 0, STDERR)
#define PERROR_CHOOSER_IMPL(_1, _2, N, ...) PERROR_##N
#define PERROR_CHOOSER(...) PERROR_CHOOSER_IMPL(__VA_ARGS__, 2, 1)

#define PERROR(...) PERROR_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define PFLUSH_TO(bufferType) flushBufferWithWriter(getWriteBuffer(bufferType))

#define PFLUSH_AFTER(bufferType)                                               \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = (PFLUSH_TO(bufferType), 1))

#endif
