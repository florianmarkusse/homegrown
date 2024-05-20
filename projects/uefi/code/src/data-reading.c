#include "data-reading.h"
#include "acpi/c-acpi-madt.h"
#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "apic.h"
#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
#include "efi/c-efi-protocol-acpi.h"
#include "efi/c-efi-protocol-block-io.h"
#include "efi/c-efi-protocol-disk-io.h"
#include "efi/c-efi-protocol-graphics-output.h"
#include "efi/c-efi-protocol-loaded-image.h"
#include "efi/c-efi-protocol-simple-file-system.h"
#include "efi/c-efi-protocol-simple-text-input.h" // for CEfiInputKey, CEfiSim...
#include "efi/c-efi-protocol-simple-text-output.h" // for CEfiSimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for CEfiSystemTable
#include "gdt.h"
#include "generated/kernel-magic.h"
#include "globals.h"
#include "kernel-parameters.h"
#include "memory/boot-functions.h"
#include "memory/definitions.h"
#include "memory/standard.h"
#include "printing.h"

AsciString readDiskLbasFromCurrentGlobalImage(CEfiLba diskLba,
                                              CEfiUSize bytes) {
    CEfiStatus status;

    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    status = globals.st->boot_services->open_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiBlockIoProtocol *imageBiop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &C_EFI_BLOCK_IO_PROTOCOL_GUID, (void **)&imageBiop,
        globals.h, C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   is the entire disk
    CEfiBlockIoProtocol *biop;
    CEfiUSize num_handles = 0;
    CEfiHandle *handle_buffer = C_EFI_NULL;

    status = globals.st->boot_services->locate_handle_buffer(
        C_EFI_BY_PROTOCOL, &C_EFI_BLOCK_IO_PROTOCOL_GUID, C_EFI_NULL,
        &num_handles, &handle_buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    CEfiHandle mediaHandle = C_EFI_NULL;
    CEfiBool readBlocks = C_EFI_FALSE;
    AsciString data;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Current Media ID: ");
    printNumber(imageBiop->Media->MediaId, 10);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    for (CEfiUSize i = 0; i < num_handles && mediaHandle == C_EFI_NULL; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &C_EFI_BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
            globals.h, C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (C_EFI_ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        CEfiU64 alignedBytes =
            ((bytes + biop->Media->BlockSize - 1) / biop->Media->BlockSize) *
            biop->Media->BlockSize;

        CEfiPhysicalAddress address;
        status = globals.st->boot_services->allocate_pages(
            C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
            BYTES_TO_PAGES(alignedBytes), &address);
        if (C_EFI_ERROR(status)) {
            error(u"Could not allocete data for disk buffer\r\n");
        }

        if (biop->Media->MediaId == imageBiop->Media->MediaId) {
            status = biop->readBlocks(biop, biop->Media->MediaId, diskLba,
                                      alignedBytes, (void *)address);
            if (!(C_EFI_ERROR(status))) {
                data = (AsciString){.buf = (CEfiChar8 *)address,
                                    .len = alignedBytes};

                const CEfiU8 kernelMagic[] = KERNEL_MAGIC;
                if (!memcmp(kernelMagic, data.buf,
                            sizeof(kernelMagic) / sizeof(kernelMagic[0]))) {
                    readBlocks = C_EFI_TRUE;
                }
            }
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(handle_buffer[i],
                                                  &C_EFI_BLOCK_IO_PROTOCOL_GUID,
                                                  globals.h, C_EFI_NULL);

        if (readBlocks) {
            break;
        }
    }

    globals.st->boot_services->close_protocol(lip->device_handle,
                                              &C_EFI_BLOCK_IO_PROTOCOL_GUID,
                                              globals.h, C_EFI_NULL);
    globals.st->boot_services->close_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, globals.h, C_EFI_NULL);

    if (!readBlocks) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    return data;
}

AsciString readDiskLbas(CEfiLba diskLba, CEfiUSize bytes, CEfiU32 mediaID) {
    CEfiStatus status;

    // Loop through and get Block IO protocol for input media ID, for entire
    // disk
    //   NOTE: This assumes the first Block IO found with logical partition
    //   is the entire disk
    CEfiBlockIoProtocol *biop;
    CEfiUSize num_handles = 0;
    CEfiHandle *handle_buffer = C_EFI_NULL;

    status = globals.st->boot_services->locate_handle_buffer(
        C_EFI_BY_PROTOCOL, &C_EFI_BLOCK_IO_PROTOCOL_GUID, C_EFI_NULL,
        &num_handles, &handle_buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    CEfiHandle mediaHandle = C_EFI_NULL;
    CEfiBool readBlocks = C_EFI_FALSE;
    AsciString data;
    for (CEfiUSize i = 0; i < num_handles && mediaHandle == C_EFI_NULL; i++) {
        status = globals.st->boot_services->open_protocol(
            handle_buffer[i], &C_EFI_BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
            globals.h, C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
        if (C_EFI_ERROR(status)) {
            error(u"Could not Open Block IO protocol on handle\r\n");
        }

        if (biop->Media->MediaId == mediaID && !biop->Media->LogicalPartition) {
            CEfiU64 alignedBytes = ((bytes + biop->Media->BlockSize - 1) /
                                    biop->Media->BlockSize) *
                                   biop->Media->BlockSize;

            CEfiPhysicalAddress address;
            status = globals.st->boot_services->allocate_pages(
                C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
                BYTES_TO_PAGES(alignedBytes), &address);
            if (C_EFI_ERROR(status)) {
                error(u"Could not allocete data for disk buffer\r\n");
            }

            status = biop->readBlocks(biop, biop->Media->MediaId, diskLba,
                                      alignedBytes, (void *)address);

            data =
                (AsciString){.buf = (CEfiChar8 *)address, .len = alignedBytes};

            readBlocks = C_EFI_TRUE;
        }

        // Close open protocol when done
        globals.st->boot_services->close_protocol(handle_buffer[i],
                                                  &C_EFI_BLOCK_IO_PROTOCOL_GUID,
                                                  globals.h, C_EFI_NULL);

        if (readBlocks) {
            break;
        }
    }

    if (!readBlocks) {
        error(u"\r\nERROR: Could not find Block IO protocol for disk with "
              u"ID\r\n");
    }

    //    // Get Disk IO Protocol on same handle as Block IO protocol
    //    CEfiDiskIOProtocol *diop;
    //    status = globals.st->boot_services->open_protocol(
    //        mediaHandle, &C_EFI_DISK_IO_PROTOCOL_GUID, (void **)&diop,
    //        globals.h, C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    //    if (C_EFI_ERROR(status)) {
    //        error(u"Could not Open Disk IO protocol on handle\r\n");
    //    }
    //
    //    CEfiPhysicalAddress address;
    //    status = globals.st->boot_services->allocate_pages(
    //        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
    //        BYTES_TO_PAGES(bytes), &address);
    //    if (C_EFI_ERROR(status)) {
    //        error(u"Could not allocete data for disk buffer\r\n");
    //    }
    //
    //    status = diop->readDisk(diop, mediaID, diskLba *
    //    biop->Media->BlockSize,
    //                            bytes, (void *)address);
    //    if (C_EFI_ERROR(status)) {
    //        error(u"Could not read Disk LBAs into buffer\r\n");
    //    }
    //
    //    // Close disk IO protocol when done
    //    globals.st->boot_services->close_protocol(
    //        mediaHandle, &C_EFI_DISK_IO_PROTOCOL_GUID, globals.h, C_EFI_NULL);

    return data;
}

CEfiU32 getDiskImageMediaID() {
    CEfiStatus status;

    // Get media ID for this disk image
    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    status = globals.st->boot_services->open_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiBlockIoProtocol *biop;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &C_EFI_BLOCK_IO_PROTOCOL_GUID, (void **)&biop,
        globals.h, C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Block IO Protocol for this loaded image.\r\n");
    }

    CEfiU32 mediaID = biop->Media->MediaId;

    globals.st->boot_services->close_protocol(lip->device_handle,
                                              &C_EFI_BLOCK_IO_PROTOCOL_GUID,
                                              globals.h, C_EFI_NULL);
    globals.st->boot_services->close_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, globals.h, C_EFI_NULL);

    return mediaID;
}

typedef enum { NAME = 0, BYTE_SIZE = 1, LBA_START = 2 } DataPartitionLayout;

DataPartitionFile getKernelInfo() {
    // Get loaded image protocol first to grab device handle to use
    //   simple file system protocol on
    CEfiLoadedImageProtocol *lip = 0;
    CEfiStatus status = globals.st->boot_services->open_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, (void **)&lip, globals.h,
        C_EFI_NULL, C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    // Get Simple File System Protocol for device handle for this loaded
    //   image, to open the root directory for the ESP
    CEfiSimpleFileSystemProtocol *sfsp = C_EFI_NULL;
    status = globals.st->boot_services->open_protocol(
        lip->device_handle, &C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
        (void **)&sfsp, globals.h, C_EFI_NULL,
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
    CEfiUSize file_info_size = sizeof(file_info);
    status =
        file->getInfo(file, &C_EFI_FILE_INFO_ID, &file_info_size, &file_info);
    if (C_EFI_ERROR(status)) {
        error(u"Could not get file info\r\n");
    }

    AsciString dataFile;
    dataFile.len = file_info.fileSize;

    CEfiPhysicalAddress dataFileAddress;

    status = globals.st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
        BYTES_TO_PAGES(dataFile.len), &dataFileAddress);
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

    globals.st->boot_services->close_protocol(
        lip->device_handle, &C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, globals.h,
        C_EFI_NULL);

    globals.st->boot_services->close_protocol(
        globals.h, &C_EFI_LOADED_IMAGE_PROTOCOL_GUID, globals.h, C_EFI_NULL);

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
