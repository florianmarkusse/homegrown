#include "data-reading.h"
#include "efi/c-efi-base.h"                        // for ERROR, Handle
#include "efi/c-efi-protocol-block-io.h"           // for BLOCK_IO_PROTOCOL...
#include "efi/c-efi-protocol-file.h"               // for FileProtocol, FIL...
#include "efi/c-efi-protocol-loaded-image.h"       // for LOADED_IMAGE_PROT...
#include "efi/c-efi-protocol-simple-file-system.h" // for SIMPLE_FILE_SYSTE...
#include "efi/c-efi-protocol-simple-text-output.h"
#include "efi/c-efi-system.h" // for OPEN_PROTOCOL_BY_...
#include "globals.h"          // for globals
#include "interoperation/generated/kernel-magic.h"
#include "interoperation/memory/definitions.h" // for BYTES_TO_PAGE_FRAMES
#include "interoperation/memory/descriptor.h"
#include "memory/standard.h" // for memcmp
#include "platform-abstraction/memory/management/virtual.h"
#include "printing.h" // for error, printNumber

AsciString readDiskLbasFromCurrentGlobalImage(Lba diskLba, USize bytes) {
    Status status;

    LoadedImageProtocol *lip = NULL;
    status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h, NULL,
        OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    BlockIoProtocol *imageBiop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&imageBiop,
        globals.h, NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   is the entire disk
    BlockIoProtocol *biop;
    USize num_handles = 0;
    Handle *handle_buffer = NULL;

    status = globals.st->boot_services->locate_handle_buffer(
        BY_PROTOCOL, &BLOCK_IO_PROTOCOL_GUID, NULL, &num_handles,
        &handle_buffer);
    if (ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    Handle mediaHandle = NULL;
    bool readBlocks = false;
    AsciString data;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Media ID UEFI loaded from: ");
    printNumber(imageBiop->Media->MediaId, 10);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    for (USize i = 0; i < num_handles && mediaHandle == NULL; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
            globals.h, NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        U64 alignedBytes =
            ((bytes + biop->Media->BlockSize - 1) / biop->Media->BlockSize) *
            biop->Media->BlockSize;

        PhysicalAddress address;
        status = globals.st->boot_services->allocate_pages(
            ALLOCATE_ANY_PAGES, LOADER_DATA, BYTES_TO_PAGE_FRAMES(alignedBytes),
            &address);
        if (ERROR(status)) {
            error(u"Could not allocete data for disk buffer\r\n");
        }

        if (biop->Media->MediaId == imageBiop->Media->MediaId) {
            status = biop->readBlocks(biop, biop->Media->MediaId, diskLba,
                                      alignedBytes, (void *)address);
            if (!(ERROR(status))) {
                data = (AsciString){.buf = (U8 *)address, .len = alignedBytes};

                const U8 kernelMagic[] = KERNEL_MAGIC;
                if (!memcmp(kernelMagic, data.buf,
                            sizeof(kernelMagic) / sizeof(kernelMagic[0]))) {
                    readBlocks = true;
                }
            }
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, globals.h, NULL);

        if (readBlocks) {
            break;
        }
    }

    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, NULL);
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, NULL);

    if (!readBlocks) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    return data;
}

AsciString readDiskLbas(Lba diskLba, USize bytes, U32 mediaID) {
    Status status;

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   is the entire disk
    BlockIoProtocol *biop;
    USize num_handles = 0;
    Handle *handle_buffer = NULL;

    status = globals.st->boot_services->locate_handle_buffer(
        BY_PROTOCOL, &BLOCK_IO_PROTOCOL_GUID, NULL, &num_handles,
        &handle_buffer);
    if (ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    Handle mediaHandle = NULL;
    bool readBlocks = false;
    AsciString data;
    for (USize i = 0; i < num_handles && mediaHandle == NULL; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
            globals.h, NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        if (biop->Media->MediaId == mediaID && !biop->Media->LogicalPartition) {
            U64 alignedBytes = ((bytes + biop->Media->BlockSize - 1) /
                                biop->Media->BlockSize) *
                               biop->Media->BlockSize;

            PhysicalAddress address;
            status = globals.st->boot_services->allocate_pages(
                ALLOCATE_ANY_PAGES, LOADER_DATA,
                BYTES_TO_PAGE_FRAMES(alignedBytes), &address);
            if (ERROR(status)) {
                error(u"Could not allocete data for disk buffer\r\n");
            }

            status = biop->readBlocks(biop, biop->Media->MediaId, diskLba,
                                      alignedBytes, (void *)address);

            data = (AsciString){.buf = (U8 *)address, .len = alignedBytes};

            readBlocks = true;
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(
            handle_buffer[i], &BLOCK_IO_PROTOCOL_GUID, globals.h, NULL);

        if (readBlocks) {
            break;
        }
    }

    if (!readBlocks) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    //    // Get Disk IO Protocol on same handle as Block IO protocol
    //    DiskIOProtocol *diop;
    //    status = globals.st->boot_services->open_protocol(
    //        mediaHandle, &DISK_IO_PROTOCOL_GUID, (void **)&diop,
    //        globals.h, NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    //    if (ERROR(status)) {
    //        error(u"Could not Open Disk IO protocol on handle\r\n");
    //    }
    //
    //    PhysicalAddress address;
    //    status = globals.st->boot_services->allocate_pages(
    //        ALLOCATE_ANY_PAGES, LOADER_DATA,
    //        BYTES_TO_PAGE_FRAMES(bytes), &address);
    //    if (ERROR(status)) {
    //        error(u"Could not allocete data for disk buffer\r\n");
    //    }
    //
    //    status = diop->readDisk(diop, mediaID, diskLba *
    //    biop->Media->BlockSize,
    //                            bytes, (void *)address);
    //    if (ERROR(status)) {
    //        error(u"Could not read Disk LBAs into buffer\r\n");
    //    }
    //
    //    // Close disk IO protocol when done
    //    globals.st->boot_services->close_protocol(
    //        mediaHandle, &DISK_IO_PROTOCOL_GUID, globals.h, NULL);

    return data;
}

U32 getDiskImageMediaID() {
    Status status;

    // Get media ID for this disk image
    LoadedImageProtocol *lip = NULL;
    status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h, NULL,
        OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    BlockIoProtocol *biop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, (void **)&biop, globals.h,
        NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    U32 mediaID = biop->Media->MediaId;

    globals.st->boot_services->close_protocol(
        lip->device_handle, &BLOCK_IO_PROTOCOL_GUID, globals.h, NULL);
    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, NULL);

    return mediaID;
}

typedef enum { NAME = 0, BYTE_SIZE = 1, LBA_START = 2 } DataPartitionLayout;

DataPartitionFile getKernelInfo() {
    // Get loaded image protocol first to grab device handle to use
    //   simple file system protocol on
    LoadedImageProtocol *lip = 0;
    Status status = globals.st->boot_services->open_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h, NULL,
        OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    // Get Simple File System Protocol for device handle for this loaded
    //   image, to open the root directory for the ESP
    SimpleFileSystemProtocol *sfsp = NULL;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void **)&sfsp,
        globals.h, NULL, OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (ERROR(status)) {
        error(u"Could not open Simple File System Protocol\r\n");
    }

    FileProtocol *root = NULL;
    status = sfsp->openVolume(sfsp, &root);
    if (ERROR(status)) {
        error(u"Could not Open Volume for root directory in ESP\r\n");
    }

    FileProtocol *file = NULL;
    status =
        root->open(root, &file, u"\\EFI\\BOOT\\DATAFLS.INF", FILE_MODE_READ, 0);
    if (ERROR(status)) {
        error(u"Could not Open File\r\n");
    }

    FileInfo file_info;
    USize file_info_size = sizeof(file_info);
    status = file->getInfo(file, &FILE_INFO_ID, &file_info_size, &file_info);
    if (ERROR(status)) {
        error(u"Could not get file info\r\n");
    }

    AsciString dataFile;
    dataFile.len = file_info.fileSize;

    PhysicalAddress dataFileAddress;

    status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA, BYTES_TO_PAGE_FRAMES(dataFile.len),
        &dataFileAddress);
    if (ERROR(status) || dataFile.len != file_info.fileSize) {
        error(u"Could not allocate memory for file\r\n");
    }

    dataFile.buf = (U8 *)dataFileAddress;

    status = file->read(file, &dataFile.len, dataFile.buf);
    if (ERROR(status) || dataFile.len != file_info.fileSize) {
        error(u"Could not read file into buffer\r\n");
    }

    root->close(root);
    file->close(file);

    globals.st->boot_services->close_protocol(
        lip->device_handle, &SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, globals.h, NULL);

    globals.st->boot_services->close_protocol(
        globals.h, &LOADED_IMAGE_PROTOCOL_GUID, globals.h, NULL);

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
                        U64 bytes = 0;
                        for (U64 i = 0; i < tokens.string.len; i++) {
                            bytes = bytes * 10 + (tokens.string.buf[i] - '0');
                        }
                        kernelFile.bytes = bytes;
                        break;
                    }
                    case LBA_START: {
                        U64 lbaStart = 0;
                        for (U64 i = 0; i < tokens.string.len; i++) {
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
