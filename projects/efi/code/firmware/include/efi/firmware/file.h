#ifndef EFI_FIRMWARE_FILE_H
#define EFI_FIRMWARE_FILE_H

#include "efi/firmware/base.h"
#include "efi/firmware/system.h"
#include "shared/uuid.h"

static constexpr auto FILE_INFO_ID =
    (UUID){.ms1 = 0x09576e92,
           .ms2 = 0x6d3f,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

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
static constexpr auto FILE_MODE_READ = 0x0000000000000001;
static constexpr auto FILE_MODE_WRITE = 0x0000000000000002;
static constexpr auto FILE_MODE_CREATE = 0x8000000000000000;

// File Attributes
static constexpr auto FILE_READ_ONLY = 0x0000000000000001;
static constexpr auto FILE_HIDDEN = 0x0000000000000002;
static constexpr auto FILE_SYSTEM = 0x0000000000000004;
static constexpr auto FILE_RESERVED = 0x0000000000000008;
static constexpr auto FILE_DIRECTORY = 0x0000000000000010;
static constexpr auto FILE_ARCHIVE = 0x0000000000000020;
static constexpr auto FILE_VALID_ATTR = 0x0000000000000037;

typedef struct FileProtocol {
    U64 Revision;
    Status(*open)(FileProtocol *this_, FileProtocol **newHandle,
                          U16 *fileName, U64 openMode, U64 attributes);
    Status(*close)(FileProtocol *this_);
    Status(*delete)(FileProtocol *this_);
    Status(*read)(FileProtocol *this_, USize *bufferSize, void *buffer);
    Status(*write)(FileProtocol *this_, USize *bufferSize,
                           void *buffer);
    Status(*getPosisition)(FileProtocol *this_, U64 position);
    Status(*setPosisition)(FileProtocol *this_, U64 position);
    Status(*getInfo)(FileProtocol *this_, UUID *informationType,
                             USize *bufferSize, void *buffer);
    Status(*setInfo)(FileProtocol *this_, UUID *informationType,
                             USize bufferSize, void *buffer);
    Status(*flush)(FileProtocol *this_);

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

#endif
