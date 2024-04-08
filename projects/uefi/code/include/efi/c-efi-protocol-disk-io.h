#ifndef EFI_C_EFI_PROTOCOL_DISK_IO_H
#define EFI_C_EFI_PROTOCOL_DISK_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_DISK_IO_PROTOCOL_GUID                                            \
    C_EFI_GUID(0xCE345171, 0xBA0B, 0x11d2, 0x8e, 0x4F, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct CEfiDiskIOProtocol {
    CEfiU64 Revision;
    CEfiUSize(CEFICALL *readDisk)(CEfiDiskIOProtocol *this_, CEfiU32 mediaId,
                                  CEfiU64 offset, CEfiUSize bufferSize,
                                  void *buffer);
    CEfiUSize(CEFICALL *writeDisk)(CEfiDiskIOProtocol *this_, CEfiU32 mediaId,
                                   CEfiU64 offset, CEfiUSize bufferSize,
                                   void *buffer);
} CEfiDiskIOProtocol;

#ifdef __cplusplus
}
#endif

#endif
