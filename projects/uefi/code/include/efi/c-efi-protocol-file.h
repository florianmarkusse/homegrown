#ifndef EFI_PROTOCOL_FILE_H
#define EFI_PROTOCOL_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-system.h"

#define FILE_INFO_ID                                                     \
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
#define FILE_MODE_READ 0x0000000000000001
#define FILE_MODE_WRITE 0x0000000000000002
#define FILE_MODE_CREATE 0x8000000000000000

// File Attributes
#define FILE_READ_ONLY 0x0000000000000001
#define FILE_HIDDEN 0x0000000000000002
#define FILE_SYSTEM 0x0000000000000004
#define FILE_RESERVED 0x0000000000000008
#define FILE_DIRECTORY 0x0000000000000010
#define FILE_ARCHIVE 0x0000000000000020
#define FILE_VALID_ATTR 0x0000000000000037

typedef struct FileProtocol {
    U64 Revision;
    Status(EFICALL *open)(FileProtocol *this_,
                               FileProtocol **newHandle,
                               U16 *fileName, U64 openMode,
                               U64 attributes);
    Status(EFICALL *close)(FileProtocol *this_);
    Status(EFICALL *delete)(FileProtocol *this_);
    Status(EFICALL *read)(FileProtocol *this_, USize *bufferSize,
                               void *buffer);
    Status(EFICALL *write)(FileProtocol *this_, USize *bufferSize,
                                void *buffer);
    Status(EFICALL *getPosisition)(FileProtocol *this_,
                                        U64 position);
    Status(EFICALL *setPosisition)(FileProtocol *this_,
                                        U64 position);
    Status(EFICALL *getInfo)(FileProtocol *this_,
                                  Guid *informationType,
                                  USize *bufferSize, void *buffer);
    Status(EFICALL *setInfo)(FileProtocol *this_,
                                  Guid *informationType,
                                  USize bufferSize, void *buffer);
    Status(EFICALL *flush)(FileProtocol *this_);

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
