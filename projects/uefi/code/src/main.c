#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
#include "efi/c-efi-protocol-block-io.h"
#include "efi/c-efi-protocol-disk-io.h"
#include "efi/c-efi-protocol-graphics-output.h"
#include "efi/c-efi-protocol-loaded-image.h"
#include "efi/c-efi-protocol-simple-file-system.h"
#include "efi/c-efi-protocol-simple-text-input.h" // for CEfiInputKey, CEfiSim...
#include "efi/c-efi-protocol-simple-text-output.h" // for CEfiSimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for CEfiSystemTable
// #include "globals.h"
// #include "memory/boot-functions.h"
// #include "memory/definitions.h"
// #include "memory/standard.h"
// #include "printing.h"
#include "memory/standard.h"

#define KERNEL_SPACE_START 0xffff800000000000
#define KERNEL_SPACE_END 0xffffffffffffffff

#define KERNEL_START 0xfffffffff8000000

#define PAGE_ENTRY_SHIFT 9
#define PAGE_ENTRY_SIZE (1 << PAGE_ENTRY_SHIFT)
#define PAGE_ENTRY_MASK (PAGE_ENTRY_SIZE - 1)

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK (PAGE_SIZE - 1)

#define PAGE_ENTRIES_SIZE_BYTES (8)
#define PAGE_ENTRIES_NUM (PAGE_SIZE / PAGE_ENTRIES_SIZE_BYTES)

#define PAGE_PRESENT (1ULL << 0)  // The page is currently in memory
#define PAGE_WRITABLE (1ULL << 1) // It’s allowed to write to this page
#define PAGE_USER_ACCESSIBLE                                                   \
    (1ULL << 2) // If not set, only kernel mode code can access this page
#define PAGE_WRITE_THROUGH (1ULL << 3) // Writes go directly to memory
#define PAGE_DISABLE_CACHE (1ULL << 4) // No cache is used for this page
#define PAGE_ACCESSED                                                          \
    (1ULL << 5) // The CPU sets this bit when this page is used
#define PAGE_DIRTY                                                             \
    (1ULL << 6) // The CPU sets this bit when a write to this page occurs
#define PAGE_HUGE_PAGE                                                         \
    (1ULL << 7) // Must be 0 in P1 and P4, creates a 1 GiB page in P3, creates a
                // 2 MiB page in P2
#define PAGE_GLOBAL                                                            \
    (1ULL << 8) // Page isn’t flushed from caches on address space switch (PGE
                // bit of CR4 register must be set)
#define PAGE_AVAILABLE_9 (1ULL << 9)   // Can be used freely by the OS
#define PAGE_AVAILABLE_10 (1ULL << 10) // Can be used freely by the OS
#define PAGE_AVAILABLE_11 (1ULL << 11) // Can be used freely by the OS
#define PAGE_PHYSICAL_ADDR_MASK                                                \
    0x7FFFFFFFFFFFFULL                 // Mask for the 52-bit physical address
#define PAGE_AVAILABLE_52 (1ULL << 52) // Can be used freely by the OS
#define PAGE_AVAILABLE_53 (1ULL << 53) // Can be used freely by the OS
#define PAGE_AVAILABLE_54 (1ULL << 54) // Can be used freely by the OS
#define PAGE_AVAILABLE_55 (1ULL << 55) // Can be used freely by the OS
#define PAGE_AVAILABLE_56 (1ULL << 56) // Can be used freely by the OS
#define PAGE_AVAILABLE_57 (1ULL << 57) // Can be used freely by the OS
#define PAGE_AVAILABLE_58 (1ULL << 58) // Can be used freely by the OS
#define PAGE_AVAILABLE_59 (1ULL << 59) // Can be used freely by the OS
#define PAGE_AVAILABLE_60 (1ULL << 60) // Can be used freely by the OS
#define PAGE_AVAILABLE_61 (1ULL << 61) // Can be used freely by the OS
#define PAGE_AVAILABLE_62 (1ULL << 62) // Can be used freely by the OS
#define PAGE_NO_EXECUTE                                                        \
    (1ULL << 63) // Forbid executing code on this page (the NXE bit in the EFER
                 // register must be set)

#define EFI_SIZE_TO_PAGES(a) (((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

CEfiHandle h;
CEfiSystemTable *st;

CEfiU64 *level4PageTable = C_EFI_NULL;

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

void print_number(CEfiUSize number, CEfiU8 base) {
    const CEfiChar16 *digits = u"0123456789ABCDEF";
    CEfiChar16 buffer[24]; // Hopefully enough for UINTN_MAX (UINT64_MAX) + sign
                           // character
    CEfiUSize i = 0;
    CEfiBool negative = C_EFI_FALSE;

    if (base > 16) {
        error(u"Invalid base specified!\r\n");
    }

    do {
        buffer[i++] = digits[number % base];
        number /= base;
    } while (number > 0);

    switch (base) {
    case 2:
        // Binary
        buffer[i++] = u'b';
        buffer[i++] = u'0';
        break;

    case 8:
        // Octal
        buffer[i++] = u'o';
        buffer[i++] = u'0';
        break;

    case 10:
        // Decimal
        if (negative)
            buffer[i++] = u'-';
        break;

    case 16:
        // Hexadecimal
        buffer[i++] = u'x';
        buffer[i++] = u'0';
        break;

    default:
        // Maybe invalid base, but we'll go with it (no special processing)
        break;
    }

    // NULL terminate string
    buffer[i--] = u'\0';

    // Reverse buffer before printing
    for (CEfiUSize j = 0; j < i; j++, i--) {
        // Swap digits
        CEfiUSize temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    // Print number string
    st->con_out->output_string(st->con_out, buffer);
}

void printHex(CEfiU64 hex) { print_number(hex, 16); }

/**
 * Allocate and zero out a page on UEFI
 */
CEfiPhysicalAddress alloc_and_zero(CEfiUSize numPages) {
    CEfiPhysicalAddress page = 0;
    CEfiStatus status = st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA, numPages, &page);
    if (C_EFI_ERROR(status)) {
        error(u"unable to allocate pages!\r\n");
    }

    memset((void *)page, 0, numPages * PAGE_SIZE);
    return page;
}

void prepareMemory(CEfiU64 phys, CEfiU64 virt, CEfiU32 size) {
    /* is this a canonical address? We handle virtual memory up to 256TB */
    if (!level4PageTable ||
        ((virt >> 48L) != 0x0000 && (virt >> 48L) != 0xffff)) {
        error(u"Incorrect address mapped or no page table set up yet!\r\n");
    }

    CEfiU64 end = virt + size;
    CEfiU64 *pageEntry = C_EFI_NULL;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_SIZE, phys += PAGE_SIZE) {
        /* 512G */
        pageEntry = &(level4PageTable[(virt >> 39L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            CEfiPhysicalAddress addr = alloc_and_zero(1);
            *pageEntry = (addr | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 1G */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 30L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (alloc_and_zero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 2M  */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 21L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (alloc_and_zero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 4K */
        pageEntry = (CEfiPhysicalAddress *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 12L) & PAGE_ENTRY_MASK]);
        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        if (!*pageEntry) {
            *pageEntry = phys | (PAGE_PRESENT | PAGE_WRITABLE);
        } else {
            error(u"This should not happen!\r\n");
        }
    }
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

    CEfiPhysicalAddress address;
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

static CEfiU8 in_exc = 0;

// Not sure what we are doing when we encounter an exception tbh.
void fw_exc(CEfiU8 excno, CEfiU64 exccode, CEfiU64 rip, CEfiU64 rsp) {
    CEfiU64 cr2, cr3;
    if (!in_exc) {
        in_exc++;
        __asm__ __volatile__("movq %%cr2, %%rax;movq %%cr3, %%rbx;"
                             : "=a"(cr2), "=b"(cr3)::);
        error(u"Ran into the first exception?\r\n");
    }
    error(u"Ran into the second exception????\r\n");
    __asm__ __volatile__("1: cli; hlt; jmp 1b");
}

typedef struct {
    CEfiUSize memoryMapSize;
    CEfiMemoryDescriptor *memoryMap;
    CEfiUSize mapKey;
    CEfiUSize descriptorSize;
    CEfiU32 descriptorVersion;
} MemoryInfo;

MemoryInfo getMemoryInfo() {
    CEfiUSize memoryMapSize = 0;
    CEfiMemoryDescriptor *memoryMap = C_EFI_NULL;
    CEfiUSize mapKey;
    CEfiUSize descriptorSize;
    CEfiU32 descriptorVersion;

    CEfiPhysicalAddress memoryMapAddress;

    // Call GetMemoryMap with initial buffer size of 0 to retrieve the
    // required buffer size
    CEfiStatus status =
        st->boot_services->get_memory_map(&memoryMapSize, memoryMap, &mapKey,
                                          &descriptorSize, &descriptorVersion);

    if (status != C_EFI_BUFFER_TOO_SMALL) {
        error(u"Should have received a buffer too small error here!\r\n");
    }

    // Some extra because allocating can create extra descriptors and
    // otherwise
    // exitbootservices will fail (lol)
    memoryMapSize += descriptorSize * 2;
    status = st->boot_services->allocate_pages(
        C_EFI_ALLOCATE_ANY_PAGES, C_EFI_LOADER_DATA,
        EFI_SIZE_TO_PAGES(memoryMapSize), &memoryMapAddress);
    if (C_EFI_ERROR(status)) {
        error(u"Could not allocate data for memory map buffer\r\n");
    }
    memoryMap = (CEfiMemoryDescriptor *)memoryMapAddress;

    status =
        st->boot_services->get_memory_map(&memoryMapSize, memoryMap, &mapKey,
                                          &descriptorSize, &descriptorVersion);
    if (C_EFI_ERROR(status)) {
        error(u"Getting memory map failed!\r\n");
    }

    return (MemoryInfo){.memoryMapSize = memoryMapSize,
                        .memoryMap = memoryMap,
                        .mapKey = mapKey,
                        .descriptorSize = descriptorSize,
                        .descriptorVersion = descriptorVersion};
}

void jumpIntoKernel() {
    CEfiGuid gop_guid = C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;

    CEfiStatus status = st->boot_services->locate_protocol(
        &gop_guid, C_EFI_NULL, (void **)&gop);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    KernelParameters params = {0};

    params.fb.columns = gop->mode->info->horizontalResolution;
    params.fb.rows = gop->mode->info->verticalResolution;
    params.fb.scanline = gop->mode->info->pixelsPerScanLine;
    params.fb.ptr = gop->mode->frameBufferBase;
    params.fb.size = gop->mode->frameBufferSize;

    st->con_out->output_string(
        st->con_out,
        u"Starting exit boot services process, no printing after this!\r\n");

    MemoryInfo memoryInfo = getMemoryInfo();
    status = st->boot_services->exit_boot_services(h, memoryInfo.mapKey);

    if (C_EFI_ERROR(status)) {
        st->con_out->output_string(st->con_out,
                                   u"First exit boot services failed..\r\n");
        printHex(status);
        print_number(status, 2);
        status = st->boot_services->free_pages(
            (CEfiPhysicalAddress)memoryInfo.memoryMap,
            EFI_SIZE_TO_PAGES(memoryInfo.memoryMapSize));
        if (C_EFI_ERROR(status)) {
            error(u"Could not free allocated memory map\r\n");
        }

        memoryInfo = getMemoryInfo();
        status = st->boot_services->exit_boot_services(h, memoryInfo.mapKey);
    }
    if (C_EFI_ERROR(status)) {
        error(u"could not exit boot services!\r\n");
    }

    /* now that we have left the firmware realm behind, we can get some real
     * work done :-) */
    __asm__ __volatile__(
        /* fw_loadseg might have altered the paging tables for higher-half
           kernels. Better to reload */
        /* CR3 to kick the MMU, but on UEFI we can only do this after we have
           called ExitBootServices */
        "movq %%rax, %%cr3;"
        /* Set up dummy exception handlers */
        ".byte 0xe8;.long 0;" /* absolute address to set the code segment
                                 register) */
        "1:popq %%rax;"
        "movq %%rax, %%rsi;addq $4f - 1b, %%rsi;" /* pointer to the code stubs
                                                   */
        "movq %%rax, %%rdi;addq $5f - 1b, %%rdi;" /* pointer to IDT */
        "addq $3f - 1b, %%rax;addq %%rax, 2(%%rax);lgdt (%%rax);" /* we must set
                                                                     up a new
                                                                     GDT with a
                                                                     TSS */
        "addq $72, %%rax;" /* patch GDT and load TR */
        "movq %%rax, %%rcx;andl $0xffffff, %%ecx;addl %%ecx, -14(%%rax);"
        "movq %%rax, %%rcx;shrq $24, %%rcx;movq %%rcx, -9(%%rax);"
        "movq $48, %%rax;ltr %%ax;"
        "movw $32, %%cx;\n" /* we set up 32 entires in IDT */
        "1:movq %%rsi, %%rax;movw $0x8F01, %%ax;shlq $16, %%rax;movw $32, "
        "%%ax;shlq $16, %%rax;movw %%si, %%ax;stosq;"
        "movq %%rsi, %%rax;shrq $32, %%rax;stosq;"
        "addq $16, %%rsi;decw %%cx;jnz 1b;" /* next entry */
        "lidt (6f);jmp 2f;"                 /* set up IDT */
        /* new GDT */
        ".balign 8;3:;"
        ".word 0x40;.long 8;.word 0;.quad 0;" /* value / null descriptor */
        ".long 0x0000FFFF;.long 0x00009800;"  /*   8 - legacy real cs */
        ".long 0x0000FFFF;.long 0x00CF9A00;"  /*  16 - prot mode cs */
        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  24 - prot mode ds */
        ".long 0x0000FFFF;.long 0x00AF9A00;"  /*  32 - long mode cs */
        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  40 - long mode ds */
        ".long 0x00000068;.long 0x00008900;"  /*  48 - long mode tss descriptor
                                               */
        ".long 0x00000000;.long 0x00000000;"  /*       cont. */
        /* TSS */
        ".long 0;.long 0x1000;.long 0;.long 0x1000;.long 0;.long 0x1000;.long "
        "0;"
        ".long 0;.long 0;.long 0x1000;.long 0;"
        /* ISRs */
        "1:popq %%r8;movq 16(%%rsp),%%r9;jmp fw_exc;"
        ".balign 16;4:xorq %%rdx, %%rdx; xorb %%cl, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $1, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $2, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $3, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $4, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $5, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $6, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $7, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $8, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $9, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $10, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $11, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $12, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $13, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $14, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $15, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $16, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $17, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $18, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $19, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $20, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $21, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $22, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $23, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $24, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $25, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $26, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $27, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $28, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $29, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $30, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $31, %%cl;jmp 1b;"
        /* IDT */
        ".balign 16;5:.space (32*16);"
        /* IDT value */
        "6:.word 6b-5b;.quad 5b;2:" ::"a"(level4PageTable)
        : "rcx", "rsi", "rdi");

    void CEFICALL (*entry_point)(KernelParameters) = (void *)KERNEL_START;
    entry_point(params);

    //    /* execute 64-bit kernels in long mode */
    //    __asm__ __volatile__(
    //        "movq %%rcx, %%r8;"
    //        /* SysV ABI uses %rdi, %rsi, but fastcall uses %rcx, %rdx */
    //        "movq %%rax, %%rcx;movq %%rax, %%rdi;"
    //        "movq %%rbx, %%rsp; movq %%rsp, %%rbp;;"
    //        "jmp *%%r8" // Jump to the address stored in %%rdx (KERNEL_START)
    //        ::"a"(&params),
    //        "b"(virtualStackPointerStart), "c"(KERNEL_START)
    //        :);

    __builtin_unreachable();
}

// TODO: table 7.10 UEFI spec section 7.2 - 7.2.1 , not fully complete yet I
// think?
bool needsTobeMappedByOS(CEfiMemoryType type) {
    switch (type) {
    case C_EFI_RUNTIME_SERVICES_DATA:
        //    case C_EFI_ACPI_RECLAIM_MEMORY:
        //    case C_EFI_ACPI_MEMORY_NVS:
        //    case C_EFI_PAL_CODE:
        return true;
    default:
        return false;
    }
}

CEFICALL CEfiStatus efi_main([[__maybe_unused__]] CEfiHandle handle,
                             CEfiSystemTable *systemtable) {
    /* make sure SSE is enabled, because some say there are buggy firmware in
     * the wild not enabling (and also needed if we come from boot_x86.asm). No
     * supported check, because according to AMD64 Spec Vol 2, all long mode
     * capable CPUs must also support SSE2 at least. We don't need them, but
     * it's more than likely that a kernel is compiled using SSE instructions.
     */
    __asm__ __volatile__(
        "movq %%cr0, %%rax;andb $0xF1, %%al;movq %%rax, %%cr0;" /* clear MP, EM,
                                                                   TS (FPU
                                                                   emulation
                                                                   off) */
        "movq %%cr4, %%rax;orw $3 << 9, %%ax;movq %%rax, %%cr4;" /* set OSFXSR,
                                                                    OSXMMEXCPT
                                                                    (enable SSE)
                                                                  */
        ::
            : "rax");

    h = handle;
    st = systemtable;

    CEfiStatus status;

    st->con_out->reset(st->con_out, false);

    st->con_out->set_attribute(st->con_out,
                               C_EFI_BACKGROUND_RED | C_EFI_YELLOW);

    level4PageTable = (CEfiPhysicalAddress *)alloc_and_zero(1);

    // Get loaded image protocol first to grab device handle to use
    //   simple file system protocol on
    CEfiGuid lip_guid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
    CEfiLoadedImageProtocol *lip = 0;
    status = st->boot_services->open_protocol(
        h, &lip_guid, (void **)&lip, h, C_EFI_NULL,
        C_EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (C_EFI_ERROR(status)) {
        error(u"Could not open Loaded Image Protocol\r\n");
    }

    // Get Simple File System Protocol for device handle for this loaded
    //   image, to open the root directory for the ESP
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

    status = st->boot_services->allocate_pages(
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

    st->boot_services->close_protocol(lip->device_handle, &sfsp_guid, h,
                                      C_EFI_NULL);

    st->boot_services->close_protocol(h, &lip_guid, h, C_EFI_NULL);

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

    st->con_out->output_string(st->con_out,
                               u"Read kernel content, at memory location:");
    printHex((CEfiUSize)kernelContent.buf);
    st->con_out->output_string(st->con_out, u"\r\n");

    MemoryInfo memoryInfo = getMemoryInfo();
    CEfiMemoryDescriptor *iterator;
    for (CEfiU64 i = 0; i < memoryInfo.memoryMapSize;
         iterator = (CEfiMemoryDescriptor *)((char *)memoryInfo.memoryMap + i),
                 i += memoryInfo.descriptorSize) {
        st->con_out->output_string(st->con_out, u"here..");
        if (needsTobeMappedByOS(iterator->type)) {
            st->con_out->output_string(st->con_out, u"Memory entry:");
            print_number(iterator->physical_start, 16);
            st->con_out->output_string(st->con_out, u" ");
            print_number(iterator->virtual_start, 16);
            st->con_out->output_string(st->con_out, u" ");
            print_number(iterator->number_of_pages, 16);
            st->con_out->output_string(st->con_out, u"\r\n");
        }
    }

    st->con_out->output_string(st->con_out,
                               u"Attempting to map memory now...\r\n");

    prepareMemory(0, 0, 4294967295); // 4 GiB ???
    prepareMemory((CEfiU64)kernelContent.buf, KERNEL_START,
                  (CEfiU32)kernelContent.len);

    st->con_out->output_string(st->con_out,
                               u"Preparing to jump to kernel...\r\n");
    jumpIntoKernel();

    return !C_EFI_SUCCESS;
}
