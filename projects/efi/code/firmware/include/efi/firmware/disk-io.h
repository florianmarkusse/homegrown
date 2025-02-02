#ifndef EFI_C_EFI_PROTOCOL_DISK_IO_H
#define EFI_C_EFI_PROTOCOL_DISK_IO_H

#include "efi/acpi/guid.h"
#include "efi/firmware/base.h"

static constexpr auto DISK_IO_PROTOCOL_GUID =
    (GUID){.ms1 = 0xCE345171,
           .ms2 = 0xBA0B,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x4F, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

typedef struct DiskIOProtocol {
    U64 Revision;
    USize(EFICALL *readDisk)(DiskIOProtocol *this_, U32 mediaId, U64 offset,
                             USize bufferSize, void *buffer);
    USize(EFICALL *writeDisk)(DiskIOProtocol *this_, U32 mediaId, U64 offset,
                              USize bufferSize, void *buffer);
} DiskIOProtocol;

#endif
