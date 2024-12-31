#ifndef IMAGE_BUILDER_UTIL_H
#define IMAGE_BUILDER_UTIL_H

#include "posix/log.h"
#include "shared/types/types.h"

void zeroRemainingBytesInLBA(U64 dataSize, WriteBuffer *file);

#endif
