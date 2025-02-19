#include "posix/log.h"
#include "abstraction/log.h"

#include <unistd.h>

#include "abstraction/memory/manipulation.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/types.h"

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

bool flushBufferWithFileDescriptor(int fileDescriptor, U8 *buffer, U64 size) {
    for (U64 bytesWritten = 0; bytesWritten < size;) {
        I64 partialBytesWritten =
            write(fileDescriptor, buffer + bytesWritten, size - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
            return false;
        } else {
            bytesWritten += partialBytesWritten;
        }
    }

    return true;
}

// We are going to flush to:
// - The in-memory standin file buffer
bool flushBufferWithWriter(WriteBuffer *buffer) {
    bool result = flushBufferWithFileDescriptor(
        buffer->fileDescriptor, buffer->array.buf, buffer->array.len);
    buffer->array.len = 0;
    return result;
}

bool flushBuffer(U8_max_a *buffer) {
    WriteBuffer writer =
        (WriteBuffer){.fileDescriptor = STDOUT_FILENO, .array = *buffer};
    bool result = flushBufferWithWriter(&writer);
    // We are copying the buffer by doing *buffer so wil l manually set lenght
    // to
    // 0
    if (result) {
        buffer->len = 0;
    }
    return result;
}

bool flushStandardBuffer() { return flushBufferWithWriter(&stdoutBuffer); }

bool handleFlags(U8 flags, WriteBuffer *buffer) {
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

// NOTE: Ready for code generation
bool appendZeroToFlushBufferWithWriter(U64 bytes, U8 flags,
                                       WriteBuffer *buffer) {
    for (U64 bytesWritten = 0; bytesWritten < bytes;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (buffer->array.cap) - buffer->array.len;
        U64 dataToWrite = bytes - bytesWritten;
        U64 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memset(buffer->array.buf + buffer->array.len, 0, bytesToWrite);
        buffer->array.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (buffer->array.cap == buffer->array.len) {
            if (!flushBufferWithWriter(buffer)) {
                return false;
            }
        }
    }

    return handleFlags(flags, buffer);
}

void appendZeroToFlushBuffer(U64 bytes, U8 flags) {
    appendZeroToFlushBufferWithWriter(bytes, flags, &stdoutBuffer);
}

// NOTE: Ready for code generation
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
        if (buffer->array.cap == buffer->array.len) {
            if (!flushBufferWithWriter(buffer)) {
                return false;
            }
        }
    }

    return handleFlags(flags, buffer);
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
