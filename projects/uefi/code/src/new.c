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

void error(CEfiU16 *string) {
    st->con_out->output_string(st->con_out, string);
    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS) {
        ;
    }
    st->runtime_services->reset_system(C_EFI_RESET_SHUTDOWN, C_EFI_SUCCESS, 0,
                                       C_EFI_NULL);
}

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK (PAGE_SIZE - 1)

#define EFI_SIZE_TO_PAGES(a) (((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

typedef struct {
    CEfiChar8 *buf;
    CEfiU64 len;
} AsciString;

#define ASCI_STRING(s) ((AsciString){(CEfiChar8 *)(s), ((sizeof(s) - 1))})

typedef struct {
    AsciString string;
    CEfiU64 pos;
} AsciStringIter;

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
    AsciString name;
    CEfiU64 bytes;
    CEfiU64 lbaStart;
} DataPartitionFile;

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

void printAsci(AsciString string) {
    CEfiChar16 print[2];
    print[1] = '\0';
    for (CEfiU64 i = 0; i < string.len; i++) {
        print[0] = string.buf[i];
        st->con_out->output_string(st->con_out, print);
    }
    st->con_out->output_string(st->con_out, u"\r\n");
}

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

    status = st->boot_services->locate_handle_buffer(
        C_EFI_BY_PROTOCOL, &bio_guid, C_EFI_NULL, &num_handles, &handle_buffer);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate any Block IO Protocols.\r\n");
    }

    CEfiHandle mediaHandle = C_EFI_NULL;
    for (CEfiUSize i = 0; i < num_handles && mediaHandle == C_EFI_NULL; i++) {
        status = st->boot_services->open_protocol(
            handle_buffer[i], &bio_guid, (void **)&biop, h, C_EFI_NULL,
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

    CEfiPhysicalAddress address = 0;
    status = st->boot_services->allocate_pages(
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
    st->boot_services->close_protocol(mediaHandle, &dio_guid, h, C_EFI_NULL);

    return (AsciString){.buf = (CEfiChar8 *)address, .len = bytes};
}

CEfiU32 getDiskImageMediaID() {
    CEfiStatus status;

    // Get media ID for this disk image
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    status = st->boot_services->open_protocol(
        h, &lip_guid, (void **)&lip, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiBlockIoProtocol *biop;
    CEfiGuid bio_guid = C_EFI_BLOCK_IO_PROTOCOL_GUID;
    status = st->boot_services->open_protocol(
        lip->device_handle, &bio_guid, (void **)&biop, h, C_EFI_NULL,
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

void jumpIntoKernel(void *kernelPtr) {
    CEfiGuid gop_guid = C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;

    CEfiStatus status = st->boot_services->locate_protocol(
        &gop_guid, C_EFI_NULL, (void **)&gop);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    CEfiPhysicalAddress gopAddress = gop->mode->frameBufferBase;
    status = st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
        EFI_SIZE_TO_PAGES(gop->mode->frameBufferSize), &gopAddress);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocete data for graphics buffer\r\n");
    }

    KernelParameters params = {0};

    params.fb.columns = gop->mode->info->horizontalResolution;
    params.fb.rows = gop->mode->info->verticalResolution;
    params.fb.scanline = gop->mode->info->pixelsPerScanLine;
    params.fb.ptr = gopAddress;
    params.fb.size = gop->mode->frameBufferSize;

    void CEFICALL (*entry_point)(KernelParameters) = kernelPtr;

    CEfiUSize memoryMapSize = 0;
    CEfiMemoryDescriptor *memoryMap = C_EFI_NULL;
    CEfiUSize mapKey;
    CEfiUSize descriptorSize;
    CEfiU32 descriptorVersion;

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    status =
        st->boot_services->get_memory_map(&memoryMapSize, memoryMap, &mapKey,
                                          &descriptorSize, &descriptorVersion);

    if (status == C_EFI_BUFFER_TOO_SMALL) {
        memoryMapSize += descriptorSize * 2;

        status = st->boot_services->allocate_pool(
            C_EFI_LOADER_DATA, memoryMapSize, (void **)&memoryMap);
        if (C_EFI_ERROR(status)) {
            error(u"Could not allocate data for memory map buffer\r\n");
        }
        status = st->boot_services->get_memory_map(&memoryMapSize, memoryMap,
                                                   &mapKey, &descriptorSize,
                                                   &descriptorVersion);
    }

    status = st->boot_services->exit_boot_services(h, mapKey);

    if (C_EFI_ERROR(status)) {
        st->con_out->output_string(st->con_out,
                                   u"First exit boot services failed..\r\n");
        status = st->boot_services->free_pool(memoryMap);
        if (C_EFI_ERROR(status)) {
            error(u"Could not free allocated memory map\r\n");
        }

        status = st->boot_services->get_memory_map(&memoryMapSize, memoryMap,
                                                   &mapKey, &descriptorSize,
                                                   &descriptorVersion);

        if (status == C_EFI_BUFFER_TOO_SMALL) {
            memoryMapSize += descriptorSize * 2;
            status = st->boot_services->allocate_pool(
                C_EFI_LOADER_DATA, memoryMapSize, (void **)&memoryMap);
            if (C_EFI_ERROR(status)) {
                error(u"Could not allocate data for memory map buffer\r\n");
            }
            status = st->boot_services->get_memory_map(
                &memoryMapSize, memoryMap, &mapKey, &descriptorSize,
                &descriptorVersion);
        }

        status = st->boot_services->exit_boot_services(h, mapKey);
    }
    if (C_EFI_ERROR(status)) {
        error(u"could not exit boot servies!\r\n");
    }

    entry_point(params);

    __builtin_unreachable();
}

CEFICALL CEfiStatus efi_main([[__maybe_unused__]] CEfiHandle handle,
                             CEfiSystemTable *sysTable) {
    h = handle;
    st = sysTable;

    st->con_out->reset(st->con_out, false);

    st->con_out->set_attribute(st->con_out,
                               C_EFI_BACKGROUND_RED | C_EFI_YELLOW);
    st->con_out->output_string(st->con_out, u"Hello from UEFI\r\n");

    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = C_EFI_NULL;
    CEfiStatus status = st->boot_services->open_protocol(
        h, &lip_guid, (void **)&lip, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    CEfiGuid sfsp_guid = C_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    CEfiSimpleFileSystemProtocol *sfsp = C_EFI_NULL;
    status = st->boot_services->open_protocol(
        lip->device_handle, &sfsp_guid, (void **)&sfsp, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Simple File System Protocol\r\n");
    }

    CEfiFileProtocol *root = C_EFI_NULL;
    status = sfsp->openVolume(sfsp, &root);
    if (C_EFI_ERROR(status)) {
        error(u"Could not Open Volume for root directory in ESP\r\n");
    }

    st->con_out->output_string(st->con_out, u"Going to read DATAFLS.INF\r\n");

    CEfiFileProtocol *file = C_EFI_NULL;
    status = root->open(root, &file, u"\\EFI\\BOOT\\DATAFLS.INF",
                        C_EFI_FILE_MODE_READ, C_EFI_FILE_READ_ONLY);
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

    CEfiUSize file_size = file_info.fileSize;
    //    print_string(u"file size: %llu\r\n", file_size);

    CEfiUSize pages =
        (file_size >> PAGE_SHIFT) + (file_size & PAGE_MASK ? 1 : 0);
    CEfiPhysicalAddress dataFileAddress;

    status = st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA, pages, &dataFileAddress);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocate for data file\r\n");
    }

    //    print_string(u"Physical address: 0x%llx - 0x%llx\r\n",
    //    phys_krnl,
    //                 phys_krnl + (pages << PAGE_SHIFT) - 1);
    //    print_string(u"Allocated %llu pages\r\n", pages);

    file->setPosition(file, 0);
    CEfiUSize read_bytes = file_size;
    // print_string(u"Attempting to read %llu bytes...", read_bytes);
    file->read(file, &read_bytes, (void *)dataFileAddress);

    AsciString dataFile =
        (AsciString){.buf = (CEfiChar8 *)dataFileAddress, .len = read_bytes};
    st->con_out->output_string(st->con_out, u"Contents of DATAFLS.INF:\r\n");
    printAsci(dataFile);

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

    if (!asciStringEquals(kernelFile.name, ASCI_STRING("kernel.bin"))) {
        error(u"kernel.bin was not read!\r\n");
    }

    st->con_out->output_string(st->con_out, u"Going to load kernel\r\n");

    AsciString kernelContent = readDiskLbas(
        kernelFile.lbaStart, kernelFile.bytes, getDiskImageMediaID());

    st->con_out->output_string(
        st->con_out, u"Read kernel content, trying to execute now!\r\n");

    printAsci(kernelContent);
    jumpIntoKernel(kernelContent.buf);

    //    // print_string(u"Read %llu bytes.\r\n", read_bytes);
    //
    //    // Clear WP flag to allow rewriting page tables
    //    // __writecr0(__readcr0() & ~CR0_WP);
    //
    //    // This is missing...
    //    // void *virt_krnl = map_kernel_and_framebuffer();
    //
    //    // __writecr0(__readcr0() | CR0_WP);
    //
    //    jumpIntoKernel((void *)phys_krnl);

    CEfiInputKey key;
    while (st->con_in->read_key_stroke(st->con_in, &key) != C_EFI_SUCCESS)
        ;

    return C_EFI_SUCCESS;
}
