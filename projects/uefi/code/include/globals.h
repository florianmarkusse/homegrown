#ifndef GLOBALS_H
#define GLOBALS_H

#include "efi/c-efi-base.h"

typedef struct {
    CEfiHandle h;
    CEfiSystemTable *st;
    CEfiU64 *level4PageTable;
    CEfiU32 maxSupportCPUID;
    CEfiU32 bootstrapProcessorID;
    CEfiU32 numberOfCores;
} Globals;

extern Globals globals;

#endif
