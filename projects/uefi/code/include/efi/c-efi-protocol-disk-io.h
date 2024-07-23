#ifndef EFI_PROTOCOL_DISK_IO_H
#define EFI_PROTOCOL_DISK_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define DISK_IO_PROTOCOL_GUID                                            \
    EFI_GUID(0xCE345171, 0xBA0B, 0x11d2, 0x8e, 0x4F, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct DiskIOProtocol {
    U64 Revision;
    USize(EFICALL *readDisk)(DiskIOProtocol *this_, U32 mediaId,
                                  U64 offset, USize bufferSize,
                                  void *buffer);
    USize(EFICALL *writeDisk)(DiskIOProtocol *this_, U32 mediaId,
                                   U64 offset, USize bufferSize,
                                   void *buffer);
} DiskIOProtocol;

#ifdef __cplusplus
}
#endif

#endif
