#include "image-builder/util.h"
#include "image-builder/configuration.h"

void zeroRemainingBytesInLBA(U64 dataSize, WriteBuffer *file) {
    if (configuration.LBASize > dataSize) {
        appendZeroToFlushBufferWithWriter(configuration.LBASize - dataSize, 0,
                                          file);
    }
}
