#ifndef EFI_FIRMWARE_SIMPLE_FILE_SYSTEM_H
#define EFI_FIRMWARE_SIMPLE_FILE_SYSTEM_H

#include "efi/firmware/base.h"
#include "efi/firmware/file.h"
#include "shared/uuid.h"

static constexpr auto SIMPLE_FILE_SYSTEM_PROTOCOL_GUID =
    (UUID){.ms1 = 0x0964e5b22,
           .ms2 = 0x6459,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

static constexpr U32 SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION = 0x00010000;

typedef struct SimpleFileSystemProtocol {
    U64 revision;
    Status(EFICALL *openVolume)(SimpleFileSystemProtocol *this_,
                                FileProtocol **Root);
} SimpleFileSystemProtocol;

#endif
