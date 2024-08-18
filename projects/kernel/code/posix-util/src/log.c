#include "util/log.h"
#include "memory/management/allocator/macros.h"
#include "util/assert.h" // for ASSERT
#include "util/maths.h"  // for MIN
#include <string.h>
#include <uchar.h>
#include <unistd.h> // for isatty, write, STDERR_FILENO, STDOUT...

#define LOG_STD_BUFFER_LEN 1 << 10
#define STRING_CONVERTER_BUF_LEN 1 << 10

static unsigned char stdoutBuf[LOG_STD_BUFFER_LEN];
static U8_max_a stdOutBuffer = {
    .buf = stdoutBuf, .cap = LOG_STD_BUFFER_LEN, .len = 0};

bool flushBuffer() {
    for (U64 bytesWritten = 0; bytesWritten < stdOutBuffer.len;) {
        U64 partialBytesWritten =
            write(STDOUT_FILENO, stdOutBuffer.buf + bytesWritten,
                  stdOutBuffer.len - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
            return false;
        } else {
            bytesWritten += partialBytesWritten;
        }
    }

    stdOutBuffer.len = 0;

    return true;
}

void appendToFlushBuffer(string data, U8 flags) {
    for (U64 bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (stdOutBuffer.cap) - stdOutBuffer.len;
        U64 dataToWrite = data.len - bytesWritten;
        U64 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memcpy(stdOutBuffer.buf + stdOutBuffer.len, data.buf + bytesWritten,
               bytesToWrite);
        stdOutBuffer.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            flushBuffer();
        }
    }

    if (flags & NEWLINE) {
        if (stdOutBuffer.len >= stdOutBuffer.cap) {
            flushBuffer();
        }
        stdOutBuffer.buf[stdOutBuffer.len] = '\n';
        stdOutBuffer.len++;
    }

    if (flags & FLUSH) {
        flushBuffer();
    }
}
