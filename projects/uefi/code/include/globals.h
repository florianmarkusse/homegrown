#ifndef GLOBALS_H
#define GLOBALS_H

#include "efi/c-efi-base.h"

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
} Globals;

extern Globals globals;

#endif
