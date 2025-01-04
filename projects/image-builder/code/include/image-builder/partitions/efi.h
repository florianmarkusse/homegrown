#ifndef IMAGE_BUILDER_PARTITIONS_EFI_H
#define IMAGE_BUILDER_PARTITIONS_EFI_H

#include "shared/types/types.h"

U32 calculateEFIPartitionSize(U32 EFIApplicationSizeLBA);
void writeEFISystemPartition(U8 *fileBuffer, U8 *efiApplicationPath);

#endif
