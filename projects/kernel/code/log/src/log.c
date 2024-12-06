#include "log/log.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/memory/sizes.h"
#include "shared/types/types.h"
#include "platform-abstraction/memory/manipulation.h"
#include "peripheral/screen/screen.h"
#include "shared/maths/maths.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * MiB);

static U8_max_a flushBuf;

void initLogger(Arena *perm) {
    flushBuf = (U8_max_a){.buf = NEW(perm, U8, FLUSH_BUFFER_SIZE),
                          .cap = FLUSH_BUFFER_SIZE,
                          .len = 0};
}

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(U8_max_a *buffer) {
    flushToScreen(*buffer);

    // TODO: Flush to file system here?

    buffer->len = 0;

    return true;
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

// TODO: buffer should be a variable to this function once we have actual
// memory management set up instead of it being hardcoded.
void appendToFlushBuffer(string data, U8 flags) {
    for (U64 bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (flushBuf.cap) - flushBuf.len;
        U64 dataToWrite = data.len - bytesWritten;
        U64 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memcpy(flushBuf.buf + flushBuf.len, data.buf + bytesWritten,
               bytesToWrite);
        flushBuf.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            flushBuffer(&flushBuf);
        }
    }

    if (flags & NEWLINE) {
        if (flushBuf.len >= flushBuf.cap) {
            flushBuffer(&flushBuf);
        }
        flushBuf.buf[flushBuf.len] = '\n';
        flushBuf.len++;
    }

    if (flags & FLUSH) {
        flushBuffer(&flushBuf);
    }
}
