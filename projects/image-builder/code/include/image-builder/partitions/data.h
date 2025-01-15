#ifndef IMAGE_BUILDER_PARTITIONS_DATA_H
#define IMAGE_BUILDER_PARTITIONS_DATA_H

#include "shared/types/types.h"

bool writeDataPartition(U8 *fileBuffer, int kernelfd, U64 kernelSizeBytes);

#endif
