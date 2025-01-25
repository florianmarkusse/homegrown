#ifndef UEFI_GUID_H
#define UEFI_GUID_H

#include "shared/uuid.h"

static constexpr UUID EFI_SYSTEM_PARTITION_GUID = {
    .timeLo = 0xC12A7328,
    .timeMid = 0xF81F,
    .timeHiAndVer = 0x11D2,
    .clockSeqHiAndRes = 0xBA,
    .clockSeqLo = 0x4B,
    .node = {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

// My data partition GUID
// Windows/Linux have their own. We could use one of theirs but where's the fun
// in that?
static constexpr UUID FLOS_BASIC_DATA_GUID = {
    .timeLo = 0x5f68a13c,
    .timeMid = 0xcdae,
    .timeHiAndVer = 0x4372,
    .clockSeqHiAndRes = 0x95,
    .clockSeqLo = 0xc7,
    .node = {0xfb, 0xc3, 0x8a, 0x42, 0xff, 0x3e}};

#endif
