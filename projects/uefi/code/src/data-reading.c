#include "data-reading.h"
#include "efi/c-efi-protocol-block-io.h"
#include "efi/c-efi-protocol-disk-io.h"
#include "efi/c-efi-protocol-loaded-image.h"
#include "efi/c-efi-protocol-simple-file-system.h"
#include "efi/c-efi-system.h"
#include "globals.h"
#include "memory/definitions.h"
#include "printing.h"

AsciString readDiskLbas(CEfiLba diskLba, CEfiUSize bytes, CEfiU32 mediaID) {
    CEfiStatus status;

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   false is the entire disk
    CEfiGuid bio_guid = C_EFI_BLOCK_IO_PROTOCOL_GUID;
    CEfiBlockIoProtocol *biop;
    CEfiUSize num_handles = 0;
    CEfiHandle *handle_buffer = C_EFI_NULL;

    status = globals.st->boot_services->locate_handle_buffer(
        C_EFI_BY_PROTOCOL, &bio_guid, C_EFI_NULL, &num_handles, &handle_buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    CEfiHandle mediaHandle = C_EFI_NULL;
    for (CEfiUSize i = 0; i < num_handles && mediaHandle == C_EFI_NULL; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &bio_guid, (void **)&biop, globals.h, C_EFI_NULL,
            C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (C_EFI_ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        if (biop->Media->MediaId == mediaID && !biop->Media->LogicalPartition) {
            mediaHandle = handle_buffer[i];
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(handle_buffer[i], &bio_guid,
                                                  globals.h, C_EFI_NULL);
    }

    if (!mediaHandle) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    // Get Disk IO Protocol on same handle as Block IO protocol
    CEfiGuid dio_guid = C_EFI_DISK_IO_PROTOCOL_GUID;
    CEfiDiskIOProtocol *diop;
    status = globals.st->boot_services->open_protocol(
        mediaHandle, &dio_guid, (void **)&diop, globals.h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not Open Disk IO protocol on handle\r\n");
    }

    CEfiPhysicalAddress address;
    status = globals.st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA, EFI_SIZE_TO_PAGES(bytes),
        &address);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocete data for disk buffer\r\n");
    }

    status = diop->readDisk(diop, mediaID, diskLba * biop->Media->BlockSize,
                            bytes, (void *)address);
    if (C_EFI_ERROR(status)) {
        error(u"Could not read Disk LBAs into buffer\r\n");
    }

    // Close disk IO protocol when done
    globals.st->boot_services->close_protocol(mediaHandle, &dio_guid, globals.h,
                                              C_EFI_NULL);

    return (AsciString){.buf = (CEfiChar8 *)address, .len = bytes};
}

CEfiU32 getDiskImageMediaID() {
    CEfiStatus status;

    // Get media ID for this disk image
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    status = globals.st->boot_services->open_protocol(
        globals.h, &lip_guid, (void **)&lip, globals.h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiBlockIoProtocol *biop;
    CEfiGuid bio_guid = C_EFI_BLOCK_IO_PROTOCOL_GUID;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &bio_guid, (void **)&biop, globals.h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    CEfiU32 mediaID = biop->Media->MediaId;

    globals.st->boot_services->close_protocol(lip->device_handle, &bio_guid,
                                              globals.h, C_EFI_NULL);
    globals.st->boot_services->close_protocol(globals.h, &lip_guid, globals.h,
                                              C_EFI_NULL);

    return mediaID;
}

typedef enum { NAME = 0, BYTE_SIZE = 1, LBA_START = 2 } DataPartitionLayout;

DataPartitionFile getKernelInfo() {
    // Get loaded image protocol first to grab device handle to use
    //   simple file system protocol on
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = 0;
    CEfiStatus status = globals.st->boot_services->open_protocol(
        globals.h, &lip_guid, (void **)&lip, globals.h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    // Get Simple File System Protocol for device handle for this loaded
    //   image, to open the root directory for the ESP
    CEfiGuid sfsp_guid = C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    CEfiSimpleFileSystemProtocol *sfsp = C_EFI_NULL;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &sfsp_guid, (void **)&sfsp, globals.h, C_EFI_NULL,
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
    status = root->open(root, &file, u"\\EFI\\BOOT\\DATAFLS.INF",
                        C_EFI_FILE_MODE_READ, 0);
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

    AsciString dataFile;
    dataFile.len = file_info.fileSize;

    CEfiPhysicalAddress dataFileAddress;

    status = globals.st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
        EFI_SIZE_TO_PAGES(dataFile.len), &dataFileAddress);
    if (C_EFI_ERROR(status) || dataFile.len != file_info.fileSize) {
        error(u"Could not allocate memory for file\r\n");
    }

    dataFile.buf = (CEfiChar8 *)dataFileAddress;

    status = file->read(file, &dataFile.len, dataFile.buf);
    if (C_EFI_ERROR(status) || dataFile.len != file_info.fileSize) {
        error(u"Could not read file into buffer\r\n");
    }

    root->close(root);
    file->close(file);

    globals.st->boot_services->close_protocol(lip->device_handle, &sfsp_guid,
                                              globals.h, C_EFI_NULL);

    globals.st->boot_services->close_protocol(globals.h, &lip_guid, globals.h,
                                              C_EFI_NULL);

    // Assumes the below file structure:
    // FILE_NAME=kernel.bin
    // FILE_SIZE=2
    // DISK_LBA=34607104
    AsciStringIter lines;
    DataPartitionFile kernelFile;
    TOKENIZE_ASCI_STRING(dataFile, lines, '\n', 0) {
        DataPartitionLayout layout = NAME;
        AsciStringIter pairs;
        TOKENIZE_ASCI_STRING(lines.string, pairs, '\t', 0) {
            AsciStringIter tokens;
            bool second = false;
            TOKENIZE_ASCI_STRING(pairs.string, tokens, '=', 0) {
                if (second) {
                    switch (layout) {
                    case NAME: {
                        if (!asciStringEquals(tokens.string,
                                              ASCI_STRING("kernel.bin"))) {
                            error(u"read file name does not equal "
                                  u"'kernel.bin'!\r\n");
                        }
                        kernelFile.name = tokens.string;
                        break;
                    }
                    case BYTE_SIZE: {
                        CEfiU64 bytes = 0;
                        for (CEfiU64 i = 0; i < tokens.string.len; i++) {
                            bytes = bytes * 10 + (tokens.string.buf[i] - '0');
                        }
                        kernelFile.bytes = bytes;
                        break;
                    }
                    case LBA_START: {
                        CEfiU64 lbaStart = 0;
                        for (CEfiU64 i = 0; i < tokens.string.len; i++) {
                            lbaStart =
                                lbaStart * 10 + (tokens.string.buf[i] - '0');
                        }
                        kernelFile.lbaStart = lbaStart;
                        break;
                    }
                    }
                }
                second = true;
            }
            layout++;
        }
    }

    return kernelFile;
}