#ifndef EFI_GLOBALS_H
#define EFI_GLOBALS_H

#include "efi/firmware/base.h"  // for Handle, SystemTable
#include "shared/types/types.h" // for U64, U32

typedef struct {
    Handle h;
    SystemTable *st;
    U64 frameBufferAddress;
} Configuration;

extern Configuration globals;

#endif
