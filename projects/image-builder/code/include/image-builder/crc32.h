#ifndef IMAGE_BUILDER_CRC32_H
#define IMAGE_BUILDER_CRC32_H

#define CRC32_TABLE_H

#include "shared/types/types.h"

U32 calculateCRC32(void *data, U64 size);

#endif
