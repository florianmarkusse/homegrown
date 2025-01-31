#ifndef EFI_DATA_READING_H
#define EFI_DATA_READING_H

#include "efi/firmware/base.h" // for Lba
#include "shared/text/string.h"
#include "shared/types/types.h" // for U32, U64, USize

typedef struct {
    U64 bytes;
    U64 lbaStart;
} DataPartitionFile;

string readDiskLbas(Lba diskLba, USize bytes, U32 mediaID);
string readDiskLbasFromCurrentGlobalImage(Lba diskLba, USize bytes);

U32 getDiskImageMediaID();
DataPartitionFile getKernelInfo();

#endif
