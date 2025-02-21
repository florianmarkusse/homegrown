#ifndef EFI_UEFI_H
#define EFI_UEFI_H

#include "shared/types/types.h"
#include "shared/uuid.h"
// Little-endian here so we swap the traditional 0x55 0xAA here
static constexpr U16 BOOT_SIGNATURE = 0xAA55;

static constexpr UUID EFI_SYSTEM_PARTITION_GUID = {
    .timeLo = 0xC12A7328,
    .timeMid = 0xF81F,
    .timeHiAndVer = 0x11D2,
    .clockSeqHiAndRes = 0xBA,
    .clockSeqLo = 0x4B,
    .node = {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

#endif
