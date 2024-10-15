#include "posix/log/log.h"
#include "interoperation/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "interoperation/memory/sizes.h"
#include "interoperation/types.h"
#include "shared/maths/maths.h"
#include "shared/memory/manipulation/manipulation.h"
#include "shared/text/string.h"
#include <unistd.h>

static constexpr auto FLUSH_BUFFER_SIZE = (2 * MiB);
static WriteBuffer stdoutBuffer =
    (WriteBuffer){.array = {.buf = (U8[FLUSH_BUFFER_SIZE]){0},
                            .cap = FLUSH_BUFFER_SIZE,
                            .len = 0},
                  .fileDescriptor = STDOUT_FILENO};
static WriteBuffer stderrBuffer =
    (WriteBuffer){.array = {.buf = (U8[FLUSH_BUFFER_SIZE]){0},
                            .cap = FLUSH_BUFFER_SIZE,
                            .len = 0},
                  .fileDescriptor = STDERR_FILENO};

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(WriteBuffer *buffer) {
    for (U64 bytesWritten = 0; bytesWritten < buffer->array.len;) {
        U64 partialBytesWritten =
            write(buffer->fileDescriptor, buffer->array.buf + bytesWritten,
                  buffer->array.len - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
            return false;
        } else {
            bytesWritten += partialBytesWritten;
        }
    }

    buffer->array.len = 0;

    return true;
}

bool appendToFlushBuffer(string data, WriteBuffer *buffer, U8 flags) {
    for (U64 bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (buffer->array.cap) - buffer->array.len;
        U64 dataToWrite = data.len - bytesWritten;
        U64 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memcpy(buffer->array.buf + buffer->array.len, data.buf + bytesWritten,
               bytesToWrite);
        buffer->array.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            if (!flushBuffer(buffer)) {
                return false;
            }
        }
    }

    if (flags & NEWLINE) {
        if (buffer->array.len >= buffer->array.cap) {
            if (!flushBuffer(buffer)) {
                return false;
            }
        }
        buffer->array.buf[buffer->array.len] = '\n';
        buffer->array.len++;
    }

    if (flags & FLUSH) {
        if (!flushBuffer(buffer)) {
            return false;
        }
    }

    return true;
}

static string ansiColorToCode[COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

bool appendColor(AnsiColor color, BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBuffer(
        isatty(buffer->fileDescriptor) ? ansiColorToCode[color] : EMPTY_STRING,
        buffer, 0);
}

bool appendColorReset(BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                   ? ansiColorToCode[COLOR_RESET]
                                   : EMPTY_STRING,
                               buffer, 0);
}

WriteBuffer *getWriteBuffer(BufferType bufferType) {
    if (bufferType == STDOUT) {
        return &stdoutBuffer;
    }
    return &stderrBuffer;
}
