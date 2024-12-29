#ifndef EFI_DATA_READING_H
#define EFI_DATA_READING_H

#include "efi/efi/c-efi-base.h" // for Lba
#include "efi/string.h"         // for AsciString
#include "shared/types/types.h" // for U32, U64, USize

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
