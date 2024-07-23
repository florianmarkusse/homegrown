#ifndef EFI_C_EFI_PROTOCOL_FILE_H
#define EFI_C_EFI_PROTOCOL_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-system.h"

#define C_EFI_FILE_INFO_ID                                                     \
    EFI_GUID(0x09576e92, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct {
    U64 size;
    U64 fileSize;
    U64 physicalSize;
    Time createTime;
    Time lastAccessTime;
    Time modificationTime;
    U64 attribute;
    // Can overflow but unlikely, oh well
    U16 fileName[256];
} FileInfo;

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

typedef struct FileProtocol {
    U64 Revision;
    Status(CEFICALL *open)(FileProtocol *this_,
                               FileProtocol **newHandle,
                               U16 *fileName, U64 openMode,
                               U64 attributes);
    Status(CEFICALL *close)(FileProtocol *this_);
    Status(CEFICALL *delete)(FileProtocol *this_);
    Status(CEFICALL *read)(FileProtocol *this_, USize *bufferSize,
                               void *buffer);
    Status(CEFICALL *write)(FileProtocol *this_, USize *bufferSize,
                                void *buffer);
    Status(CEFICALL *getPosisition)(FileProtocol *this_,
                                        U64 position);
    Status(CEFICALL *setPosisition)(FileProtocol *this_,
                                        U64 position);
    Status(CEFICALL *getInfo)(FileProtocol *this_,
                                  Guid *informationType,
                                  USize *bufferSize, void *buffer);
    Status(CEFICALL *setInfo)(FileProtocol *this_,
                                  Guid *informationType,
                                  USize bufferSize, void *buffer);
    Status(CEFICALL *flush)(FileProtocol *this_);

    // the functions below are not (yet) implemented
    // EFI_FILE_OPEN_EX  OpenEx;  // Added for revision 2
    void *OpenEx;
    // EFI_FILE_READ_EX  ReadEx;  // Added for revision 2
    void *ReadEx;
    // EFI_FILE_WRITE_EX WriteEx; // Added for revision 2
    void *WriteEx;
    // EFI_FILE_FLUSH_EX FlushEx; // Added for revision 2
    void *FlushEx;
} FileProtocol;

#ifdef __cplusplus
}
#endif

#endif
