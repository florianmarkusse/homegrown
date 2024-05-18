#ifndef GLOBALS_H
#define GLOBALS_H

#include "efi/c-efi-base.h"

typedef struct {
    CEfiHandle h;
    CEfiSystemTable *st;
    CEfiU64 level4PageTable;
    CEfiU64 highestStackAddress; // For all core stacks.
    CEfiU32 maxSupportCPUID;
    CEfiU64 bootstrapProcessorID;
    CEfiU64 numberOfCores;
    CEfiU32 processorVersionInfo;
    CEfiU32 ecxFeatureInfo;
    CEfiU32 edxFeatureInfo;
    CEfiU64 frameBufferAddress;
} Globals;

extern Globals globals;

#endif
