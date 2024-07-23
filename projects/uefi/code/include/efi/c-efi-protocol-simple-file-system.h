#ifndef EFI_C_EFI_PROTOCOL_SIMPLE_FILE_SYSTEM_H
#define EFI_C_EFI_PROTOCOL_SIMPLE_FILE_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-protocol-file.h"

#define C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID                                 \
    EFI_GUID(0x0964e5b22, 0x6459, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9,      \
               0x69, 0x72, 0x3b)

#define C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION U32_C(0x00010000)

typedef struct SimpleFileSystemProtocol {
    U64 revision;
    Status(CEFICALL *openVolume)(SimpleFileSystemProtocol *this_,
                                     FileProtocol **Root);
} SimpleFileSystemProtocol;

#ifdef __cplusplus
}
#endif

#endif
