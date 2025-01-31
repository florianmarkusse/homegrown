#ifndef UEFI_CONSTANTS_H
#define UEFI_CONSTANTS_H

#include "shared/types/types.h"
// Little-endian here so we swap the traditional 0x55 0xAA here
static constexpr U16 BOOT_SIGNATURE = 0xAA55;

#endif
