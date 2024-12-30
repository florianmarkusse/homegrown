#ifndef UEFI_GUID_H
#define UEFI_GUID_H

#include "shared/uuid.h"

static constexpr UUID EFI_SYSTEM_PARTITION_GUID = {
    .first32 = 0xC12A7328,
    .from32To47 = 0xF81F,
    .from48To63 = 0x11D2,
    .from64To79 = 0xBA4B,
    .from80to127 = {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

// My data partition GUID
// Windows/Linux have their own. We could use one of theirs but where's the fun
// in that?
static constexpr UUID FLOS_BASIC_DATA_GUID = {
    .first32 = 0x5f68a13c,
    .from32To47 = 0xcdae,
    .from48To63 = 0x4372,
    .from64To79 = 0x95c7,
    .from80to127 = {0xfb, 0xc3, 0x8a, 0x42, 0xff, 0x3e}};

#endif
