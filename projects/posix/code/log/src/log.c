#include "posix/log.h"

#include <unistd.h>

#include "platform-abstraction/memory/manipulation.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/types.h"
#include "shared/assert.h"

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
// - The in-memory standin file buffer
bool flushBufferWithWriter(WriteBuffer *buffer) {
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

bool flushBuffer(U8_max_a *buffer) {
    WriteBuffer writer =
        (WriteBuffer){.fileDescriptor = STDOUT_FILENO, .array = *buffer};
    bool result = flushBufferWithWriter(&writer);
    // We are copying the buffer by doing *buffer so will manually set lenght to
    // 0
    if (result) {
        buffer->len = 0;
    }
    return result;
}

bool flushStandardBuffer() { return flushBufferWithWriter(&stdoutBuffer); }

bool appendToFlushBufferWithWriter(string data, U8 flags, WriteBuffer *buffer) {
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
            if (!flushBufferWithWriter(buffer)) {
                return false;
            }
        }
    }

    if (flags & NEWLINE) {
        if (buffer->array.len >= buffer->array.cap) {
            if (!flushBufferWithWriter(buffer)) {
                return false;
            }
        }
        buffer->array.buf[buffer->array.len] = '\n';
        buffer->array.len++;
    }

    if (flags & FLUSH) {
        if (!flushBufferWithWriter(buffer)) {
            return false;
        }
    }

    return true;
}

void appendToFlushBuffer(string data, U8 flags) {
    appendToFlushBufferWithWriter(data, flags, &stdoutBuffer);
}

static string ansiColorToCode[COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

bool appendColor(AnsiColor color, BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBufferWithWriter(
        isatty(buffer->fileDescriptor) ? ansiColorToCode[color] : EMPTY_STRING,
        0, buffer);
}

bool appendColorReset(BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBufferWithWriter(isatty(buffer->fileDescriptor)
                                             ? ansiColorToCode[COLOR_RESET]
                                             : EMPTY_STRING,
                                         0, buffer);
}

WriteBuffer *getWriteBuffer(BufferType bufferType) {
    if (bufferType == STDOUT) {
        return &stdoutBuffer;
    }
    return &stderrBuffer;
}
