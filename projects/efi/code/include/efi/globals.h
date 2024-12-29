#ifndef EFI_GLOBALS_H
#define EFI_GLOBALS_H

#include "efi/efi/c-efi-base.h" // for Handle, SystemTable
#include "shared/types/types.h" // for U64, U32

typedef struct {
    Handle h;
    SystemTable *st;
    U64 level4PageTable;
    U64 highestStackAddress; // For all core stacks.
    U32 maxSupportCPUID;
    U64 bootstrapProcessorID;
    U64 numberOfCores;
    U32 processorVersionInfo;
    U32 ecxFeatureInfo;
    U32 edxFeatureInfo;
    U64 frameBufferAddress;
} Configuration;

extern Configuration globals;

#endif
