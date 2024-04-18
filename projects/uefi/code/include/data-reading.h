#ifndef DATA_READING_H
#define DATA_READING_H

#include "efi/c-efi-base.h"
#include "string.h"

typedef struct {
    AsciString name;
    CEfiU64 bytes;
    CEfiU64 lbaStart;
} DataPartitionFile;

AsciString readDiskLbas(CEfiLba diskLba, CEfiUSize bytes, CEfiU32 mediaID);
CEfiU32 getDiskImageMediaID();
DataPartitionFile getKernelInfo();

#endif
