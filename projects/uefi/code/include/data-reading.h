#ifndef DATA_READING_H
#define DATA_READING_H

#include "efi/c-efi-base.h"
#include "string.h"

typedef struct {
    AsciString name;
    U64 bytes;
    U64 lbaStart;
} DataPartitionFile;

AsciString readDiskLbas(Lba diskLba, USize bytes, U32 mediaID);
AsciString readDiskLbasFromCurrentGlobalImage(Lba diskLba, USize bytes);

U32 getDiskImageMediaID();
DataPartitionFile getKernelInfo();

#endif
