#ifndef EFI_C_EFI_PROTOCOL_FILE_H
#define EFI_C_EFI_PROTOCOL_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-system.h"

#define C_EFI_FILE_INFO_ID                                                     \
    C_EFI_GUID(0x09576e92, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct {
    U64 size;
    U64 fileSize;
    U64 physicalSize;
    CEfiTime createTime;
    CEfiTime lastAccessTime;
    CEfiTime modificationTime;
    U64 attribute;
    // Can overflow but unlikely, oh well
    CEfiChar16 fileName[256];
} CEfiFileInfo;

// Open Modes
#define C_EFI_FILE_MODE_READ 0x0000000000000001
#define C_EFI_FILE_MODE_WRITE 0x0000000000000002
#define C_EFI_FILE_MODE_CREATE 0x8000000000000000

// File Attributes
#define C_EFI_FILE_READ_ONLY 0x0000000000000001
#define C_EFI_FILE_HIDDEN 0x0000000000000002
#define C_EFI_FILE_SYSTEM 0x0000000000000004
#define C_EFI_FILE_RESERVED 0x0000000000000008
#define C_EFI_FILE_DIRECTORY 0x0000000000000010
#define C_EFI_FILE_ARCHIVE 0x0000000000000020
#define C_EFI_FILE_VALID_ATTR 0x0000000000000037

typedef struct CEfiFileProtocol {
    U64 Revision;
    CEfiStatus(CEFICALL *open)(CEfiFileProtocol *this_,
                               CEfiFileProtocol **newHandle,
                               CEfiChar16 *fileName, U64 openMode,
                               U64 attributes);
    CEfiStatus(CEFICALL *close)(CEfiFileProtocol *this_);
    CEfiStatus(CEFICALL *delete)(CEfiFileProtocol *this_);
    CEfiStatus(CEFICALL *read)(CEfiFileProtocol *this_, CEfiUSize *bufferSize,
                               void *buffer);
    CEfiStatus(CEFICALL *write)(CEfiFileProtocol *this_, CEfiUSize *bufferSize,
                                void *buffer);
    CEfiStatus(CEFICALL *getPosisition)(CEfiFileProtocol *this_,
                                        U64 position);
    CEfiStatus(CEFICALL *setPosisition)(CEfiFileProtocol *this_,
                                        U64 position);
    CEfiStatus(CEFICALL *getInfo)(CEfiFileProtocol *this_,
                                  CEfiGuid *informationType,
                                  CEfiUSize *bufferSize, void *buffer);
    CEfiStatus(CEFICALL *setInfo)(CEfiFileProtocol *this_,
                                  CEfiGuid *informationType,
                                  CEfiUSize bufferSize, void *buffer);
    CEfiStatus(CEFICALL *flush)(CEfiFileProtocol *this_);

    // the functions below are not (yet) implemented
    // EFI_FILE_OPEN_EX  OpenEx;  // Added for revision 2
    void *OpenEx;
    // EFI_FILE_READ_EX  ReadEx;  // Added for revision 2
    void *ReadEx;
    // EFI_FILE_WRITE_EX WriteEx; // Added for revision 2
    void *WriteEx;
    // EFI_FILE_FLUSH_EX FlushEx; // Added for revision 2
    void *FlushEx;
} CEfiFileProtocol;

#ifdef __cplusplus
}
#endif

#endif
