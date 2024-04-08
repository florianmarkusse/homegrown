#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
#include "efi/c-efi-protocol-block-io.h"
#include "efi/c-efi-protocol-disk-io.h"
#include "efi/c-efi-protocol-graphics-output.h"
#include "efi/c-efi-protocol-loaded-image.h"
#include "efi/c-efi-protocol-simple-file-system.h"
#include "efi/c-efi-protocol-simple-text-input.h" // for CEfiInputKey, CEfiSim...
#include "efi/c-efi-protocol-simple-text-output.h" // for CEfiSimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for CEfiSystemTable
#include "memory.h"

CEfiHandle h;
CEfiSystemTable *st;

typedef struct {
    CEfiChar8 *buf;
    CEfiU64 len;
} AsciString;

#define ASCI_STRING(s) ((AsciString){(CEfiChar8 *)(s), ((sizeof(s) - 1))})

typedef struct {
    AsciString string;
    CEfiU64 pos;
} AsciStringIter;

static inline bool asciStringEquals(AsciString a, AsciString b) {
    return a.len == b.len && (a.len == 0 || !memcmp(a.buf, b.buf, a.len));
}

#define TOKENIZE_ASCI_STRING(_string, stringIter, token, startingPosition)     \
    for ((stringIter) =                                                        \
             (AsciStringIter){                                                 \
                 .string = splitString(_string, token, startingPosition),      \
                 .pos = (startingPosition)};                                   \
         (stringIter).pos < (_string).len;                                     \
         (stringIter).pos += (stringIter).string.len + 1,                      \
        (stringIter).string = splitString(_string, token, (stringIter).pos))

typedef enum { NAME = 0, BYTE_SIZE = 1, LBA_START = 2 } DataPartitionLayout;

typedef struct {
    CEfiChar16 *buf;
    CEfiU64 len;
} Utf16String;

#define UTF16_STRING(s)                                                        \
    ((Utf16String){(CEfiChar16 *)(s), ((sizeof(s) / 2) - 1)})

typedef struct {
    AsciString name;
    CEfiU64 bytes;
    CEfiU64 lbaStart;
} DataPartitionFile;

static inline CEfiChar8 *getPtr(AsciString str, CEfiU64 index) {
    return &str.buf[index];
}

static inline AsciString splitString(AsciString s, CEfiChar8 token,
                                     CEfiU64 from) {
    for (CEfiU64 i = from; i < s.len; i++) {
        if (s.buf[i] == token) {
            return (AsciString){.buf = getPtr(s, from), .len = i - from};
        }
    }

    return (AsciString){.buf = getPtr(s, from), .len = s.len - from};
}

void error(CEfiU16 *string) {
    st->con_out->output_string(st->con_out, string);
    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS) {
        ;
    }
    st->runtime_services->reset_system(C_EFI_RESET_SHUTDOWN, C_EFI_SUCCESS, 0,
                                       C_EFI_NULL);
}

typedef struct {
    CEfiU32 columns;
    CEfiU32 rows;
    CEfiU32 scanline;
    CEfiU64 ptr;
    CEfiU64 size;
} FrameBuffer;

typedef struct {
    CEfiU64 ptr;
    CEfiU64 size;
} MemoryMap;

typedef struct {
    FrameBuffer fb;
    MemoryMap *memory;
} KernelParameters;

void *readDiskLbas(CEfiLba diskLba, CEfiUSize bytes, CEfiU32 mediaID) {
    CEfiStatus status;

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   false is the entire disk
    CEfiGuid bio_guid = C_EFI_BLOCK_IO_PROTOCOL_GUID;
    CEfiBlockIoProtocol *biop;
    CEfiUSize num_handles = 0;
    CEfiHandle *handle_buffer = C_EFI_NULL;

    status = st->boot_services->locate_handle_buffer(
        C_EFI_BY_PROTOCOL, &bio_guid, C_EFI_NULL, &num_handles, &handle_buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    CEfiHandle mediaHandle = C_EFI_NULL;
    for (CEfiUSize i = 0; i < num_handles && mediaHandle == C_EFI_NULL; i++) {
        status = st->boot_services->open_protocol(
            handle_buffer[i], &bio_guid, (void *)&biop, h, C_EFI_NULL,
            C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (C_EFI_ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        if (biop->Media->MediaId == mediaID && !biop->Media->LogicalPartition) {
            mediaHandle = handle_buffer[i];
        }

        // Close open protocol when done
        st->boot_services->close_protocol(handle_buffer[i], &bio_guid, h,
                                          C_EFI_NULL);
    }

    if (!mediaHandle) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    // Get Disk IO Protocol on same handle as Block IO protocol
    CEfiGuid dio_guid = C_EFI_DISK_IO_PROTOCOL_GUID;
    CEfiDiskIOProtocol *diop;
    status = st->boot_services->open_protocol(
        mediaHandle, &dio_guid, (void **)&diop, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not Open Disk IO protocol on handle\r\n");
    }

    void *buffer;
    status =
        st->boot_services->allocate_pool(C_EFI_LOADER_DATA, bytes, &buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocete data for disk buffer\r\n");
    }

    status = diop->readDisk(diop, mediaID, diskLba * biop->Media->BlockSize,
                            bytes, buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not read Disk LBAs into buffer\r\n");
    }

    // Close disk IO protocol when done
    st->boot_services->close_protocol(mediaHandle, &dio_guid, h, C_EFI_NULL);

    return buffer;
}

CEfiU32 getDiskImageMediaID() {
    CEfiStatus status;

    // Get media ID for this disk image
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    status = st->boot_services->open_protocol(
        h, &lip_guid, (void *)&lip, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiBlockIoProtocol *biop;
    CEfiGuid bio_guid = C_EFI_BLOCK_IO_PROTOCOL_GUID;
    status = st->boot_services->open_protocol(
        lip->device_handle, &bio_guid, (void *)&biop, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    CEfiU32 mediaID = biop->Media->MediaId;

    st->boot_services->close_protocol(lip->device_handle, &bio_guid, h,
                                      C_EFI_NULL);
    st->boot_services->close_protocol(h, &lip_guid, h, C_EFI_NULL);

    return mediaID;
}

AsciString readEspFile(CEfiU16 *path) {
    CEfiStatus status;
    // Get loaded image protocol first to grab device handle to use
    //   simple file system protocol on
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = 0;
    status = st->boot_services->open_protocol(
        h, &lip_guid, (void *)&lip, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    // Get Simple File System Protocol for device handle for this loaded
    //   image, to open the root directory for the ESP
    CEfiGuid sfsp_guid = C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    CEfiSimpleFileSystemProtocol *sfsp = C_EFI_NULL;
    status = st->boot_services->open_protocol(
        lip->device_handle, &sfsp_guid, (void *)&sfsp, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Simple File System Protocol\r\n");
    }

    CEfiFileProtocol *root = C_EFI_NULL;
    status = sfsp->openVolume(sfsp, &root);
    if (C_EFI_ERROR(status)) {
        error(u"Could not Open Volume for root directory in ESP\r\n");
    }

    CEfiFileProtocol *file = C_EFI_NULL;
    status = root->open(root, &file, path, C_EFI_FILE_MODE_READ, 0);
    if (C_EFI_ERROR(status)) {
        error(u"Could not Open File\r\n");
    }

    CEfiFileInfo file_info;
    CEfiGuid fi_guid = C_EFI_FILE_INFO_ID;
    CEfiUSize file_info_size = sizeof(file_info);
    status = file->getInfo(file, &fi_guid, &file_info_size, &file_info);
    if (C_EFI_ERROR(status)) {
        error(u"Could not get file info\r\n");
    }

    AsciString result;
    result.len = file_info.fileSize;

    status = st->boot_services->allocate_pool(C_EFI_LOADER_DATA, result.len,
                                              (void *)&result.buf);
    if (C_EFI_ERROR(status) || result.len != file_info.fileSize) {
        error(u"Could not allocate memory for file\r\n");
    }

    status = file->read(file, &result.len, result.buf);
    if (C_EFI_ERROR(status) || result.len != file_info.fileSize) {
        error(u"Could not read file into buffer\r\n");
    }

    root->close(root);
    file->close(file);

    st->boot_services->close_protocol(lip->device_handle, &sfsp_guid, h,
                                      C_EFI_NULL);

    st->boot_services->close_protocol(h, &lip_guid, h, C_EFI_NULL);

    return result;
}

void printAsciString(AsciString string) {
    unsigned char *pos = (unsigned char *)string.buf;
    for (CEfiUSize bytes = string.len; bytes > 0; bytes--) {
        CEfiChar16 str[2];
        str[0] = *pos;
        str[1] = u'\0';
        if (*pos == '\n') {
            st->con_out->output_string(st->con_out, u"\r\n");
        } else {
            st->con_out->output_string(st->con_out, str);
        }

        pos++;
    }
    st->con_out->output_string(st->con_out, u"\r\n");
}

CEFICALL CEfiStatus efi_main([[__maybe_unused__]] CEfiHandle handle,
                             CEfiSystemTable *systemtable) {
    h = handle;
    st = systemtable;

    st->con_out->reset(st->con_out, false);

    st->con_out->set_attribute(st->con_out,
                               C_EFI_BACKGROUND_RED | C_EFI_YELLOW);

    AsciString espFile = readEspFile(u"\\EFI\\BOOT\\DATAFLS.INF");

    // Assumes the below file structure:
    // FILE_NAME=kernel.bin
    // FILE_SIZE=2
    // DISK_LBA=34607104
    // FILE_NAME=other.txt
    // FILE_SIZE=5
    // DISK_LBA=34607156
    AsciStringIter lines;
    TOKENIZE_ASCI_STRING(espFile, lines, '\n', 0) {
        DataPartitionFile dataFile;
        DataPartitionLayout layout = NAME;
        AsciStringIter pairs;
        TOKENIZE_ASCI_STRING(lines.string, pairs, '\t', 0) {
            AsciStringIter tokens;
            bool second = false;
            TOKENIZE_ASCI_STRING(pairs.string, tokens, '=', 0) {
                if (second) {
                    switch (layout) {
                    case NAME: {
                        dataFile.name = tokens.string;
                        break;
                    }
                    case BYTE_SIZE: {
                        CEfiU64 bytes = 0;
                        for (CEfiU64 i = 0; i < tokens.string.len; i++) {
                            bytes = bytes * 10 + (tokens.string.buf[i] - '0');
                        }
                        dataFile.bytes = bytes;
                        break;
                    }
                    case LBA_START: {
                        CEfiU64 lbaStart = 0;
                        for (CEfiU64 i = 0; i < tokens.string.len; i++) {
                            lbaStart =
                                lbaStart * 10 + (tokens.string.buf[i] - '0');
                        }
                        dataFile.lbaStart = lbaStart;
                        break;
                    }
                    }
                }
                second = true;
            }
            layout++;
        }

        if (asciStringEquals(dataFile.name, ASCI_STRING("kernel.bin"))) {
            void *kernelContent = readDiskLbas(
                dataFile.lbaStart, dataFile.bytes, getDiskImageMediaID());

            CEfiGuid gop_guid = C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
            CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;

            CEfiStatus status = st->boot_services->locate_protocol(
                &gop_guid, C_EFI_NULL, (void *)&gop);
            if (C_EFI_ERROR(status)) {
                error(u"Could not locate locate GOP\r\n");
            }

            KernelParameters params = {0};

            params.fb.columns = gop->mode->info->horizontalResolution;
            params.fb.rows = gop->mode->info->verticalResolution;
            params.fb.scanline = gop->mode->info->pixelsPerScanLine;
            params.fb.ptr = gop->mode->frameBufferBase;
            params.fb.size = gop->mode->frameBufferSize;

            CEfiU32 *ptr = (CEfiU32 *)params.fb.ptr;
            ptr[0] = 0xFFDDDDDD;
            ptr[1] = 0xFFDDDDDD;
            ptr[2] = 0xFFDDDDDD;
            ptr[3] = 0xFFDDDDDD;

            void CEFICALL (*entry_point)(KernelParameters) = kernelContent;

            CEfiUSize memoryMapSize = 0;
            CEfiMemoryDescriptor *memoryMap = C_EFI_NULL;
            CEfiUSize mapKey;
            CEfiUSize descriptorSize;
            CEfiU32 descriptorVersion;

            // Call GetMemoryMap with initial buffer size of 0 to retrieve the
            // required buffer size
            status = st->boot_services->get_memory_map(
                &memoryMapSize, memoryMap, &mapKey, &descriptorSize,
                &descriptorVersion);
            if (status == C_EFI_SUCCESS) {
                error(u"Error calling initial memory map, should have failed "
                      u"initially...\r\n");
            }

            // Allocating can in and of itself result in 1/2 more memory
            // descriptors so we are playing it safe.
            memoryMapSize += descriptorSize * 2;

            status = st->boot_services->allocate_pool(
                C_EFI_LOADER_DATA, memoryMapSize, (void *)&memoryMap);
            if (C_EFI_ERROR(status)) {
                error(u"Could not allocete data for memory map buffer\r\n");
            }

            status = st->boot_services->get_memory_map(
                &memoryMapSize, memoryMap, &mapKey, &descriptorSize,
                &descriptorVersion);
            if (C_EFI_ERROR(status)) {
                error(u"Error calling second memory map\r\n");
            }

            status = st->boot_services->exit_boot_services(h, mapKey);
            if (C_EFI_ERROR(status)) {
                error(u"Error exiting boot services\r\n");
            }
            entry_point(params);

            __builtin_unreachable();
        }
    }

    st->con_out->output_string(st->con_out, u"Success...\r\n");

    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS)
        ;

    return C_EFI_SUCCESS;
}
