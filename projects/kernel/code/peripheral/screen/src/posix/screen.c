#include "peripheral/screen/screen.h"
#include "interoperation/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "util/assert.h"                // for ASSERT
#include <unistd.h>

void initScreen([[maybe_unused]] ScreenDimension dimension) {
    // Stubs for now.
}
void rewind([[maybe_unused]] U16 numberOfScreenLines) {
    // Stubs for now.
}
void prowind([[maybe_unused]] U16 numberOfScreenLines) {
    // Stubs for now.
}

bool flushToScreen(U8_max_a buffer) {
    for (U64 bytesWritten = 0; bytesWritten < buffer.len;) {
        U64 partialBytesWritten =
            write(STDOUT_FILENO, buffer.buf + bytesWritten,
                  buffer.len - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
            return false;
        } else {
            bytesWritten += partialBytesWritten;
        }
    }

    return true;
}
