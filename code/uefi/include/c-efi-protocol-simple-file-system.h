#ifdef __cplusplus
extern "C" {
#endif

#include <c-efi-base.h>
#include <c-efi-protocol-file.h>
#include <c-efi-system.h>

#define C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID                                 \
    C_EFI_GUID(0x0964e5b22, 0x6459, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9,      \
               0x69, 0x72, 0x3b)

#define C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION C_EFI_U32_C(0x00010000)

typedef struct CEfiSimpleFileSystemProtocol {
    CEfiU64 revision;
    CEfiStatus(CEFICALL *openVolume)(CEfiSimpleFileSystemProtocol *this_,
                                     CEfiFileProtocol **Root);
} CEfiSimpleFileSystemProtocol;

#ifdef __cplusplus
}
#endif
