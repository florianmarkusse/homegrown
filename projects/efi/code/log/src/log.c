#include "platform-abstraction/log.h"

#include "efi/firmware/simple-text-output.h"
#include "efi/firmware/system.h"
#include "efi/globals.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "shared/types/types.h"

static constexpr auto FLUSH_BUFFER_SIZE = (2 * KiB);
U8_max_a flushBuf = (U8_max_a){
    .buf = (U8[FLUSH_BUFFER_SIZE]){0}, .len = 0, .cap = FLUSH_BUFFER_SIZE};

static U16 convertedChar[2] = {0, '\0'};
bool flushBuffer(U8_max_a *buffer) {
    for (U64 i = 0; i < buffer->len; i++) {
        if (buffer->buf[i] == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            convertedChar[0] = buffer->buf[i];
            globals.st->con_out->output_string(globals.st->con_out,
                                               convertedChar);
        }
    }

    buffer->len = 0;
    return true;
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

void handleFlags(U8 flags) {
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

// NOTE: Ready for code generation
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
        if (flushBuf.cap == flushBuf.len) {
            flushBuffer(&flushBuf);
        }
    }

    handleFlags(flags);
}

// NOTE: Ready for code generation
void appendZeroToFlushBuffer(U64 bytes, U8 flags) {
    for (U64 bytesWritten = 0; bytesWritten < bytes;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (flushBuf.cap) - flushBuf.len;
        U64 dataToWrite = bytes - bytesWritten;
        U64 bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memset(flushBuf.buf + flushBuf.len, 0, bytesToWrite);
        flushBuf.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (flushBuf.cap == flushBuf.len) {
            flushBuffer(&flushBuf);
        }
    }

    handleFlags(flags);
}
