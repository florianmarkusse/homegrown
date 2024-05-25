#include "acpi/c-acpi-madt.h"
#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "apic.h"
#include "data-reading.h"
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
#include "globals.h"
#include "kernel-parameters.h"
#include "memory/boot-functions.h"
#include "memory/definitions.h"
#include "memory/standard.h"
#include "printing.h"
#include "string.h"

extern void ap_trampoline();
CEfiU16 lapic_ids[1024];
CEfiU64 lapic_addr = 0;

CEfiU64 *paging; // paging table for MMU

typedef enum {
    DefaultParity,
    NoParity,
    EvenParity,
    OddParity,
    MarkParity,
    SpaceParity
} EFI_PARITY_TYPE;

typedef enum {
    DefaultStopBits,
    OneStopBit,      // 1 stop bit
    OneFiveStopBits, // 1.5 stop bits
    TwoStopBits      // 2 stop bits
} EFI_STOP_BITS_TYPE;

/*** other defines and structs ***/
typedef struct {
    CEfiU8 magic[8];
    CEfiU8 chksum;
    CEfiChar8 oemid[6];
    CEfiU8 revision;
    CEfiU32 rsdt;
    CEfiU32 length;
    CEfiU64 xsdt;
    CEfiU32 echksum;
} __attribute__((packed)) ACPI_RSDPTR;

#define PAGESIZE 4096

/**
 * return type for fs drivers
 */
typedef struct {
    CEfiU8 *ptr;
    CEfiUSize size;
} file_t;

/*** common variables ***/
// file_t env;         // environment file descriptor
// file_t initrd;      // initrd file descriptor
// file_t core;        // kernel file descriptor
// BOOTBOOT *bootboot; // the BOOTBOOT structure
// CEfiU64 *paging;     // paging table for MMU
// CEfiU64 entrypoint;  // kernel entry point
// CEfiU64 fb_addr = BOOTBOOT_FB;       // virtual addresses
// CEfiU64 bb_addr = BOOTBOOT_INFO;
// CEfiU64 env_addr= BOOTBOOT_ENV;
// CEfiU64 core_addr=BOOTBOOT_CORE;

// EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
// EFI_FILE_HANDLE                 RootDir;
// EFI_FILE_PROTOCOL               *Root;
// SIMPLE_INPUT_INTERFACE          *CI;
unsigned char *kne, nosmp = 0;
volatile char bsp_done = 0, ap_done = 0;

// default environment variables. M$ states that 1024x768 must be supported
int reqwidth = 1024, reqheight = 768;
char *kernelname = "sys/core";

// alternative environment name
char *cfgname = "sys/config";

/**
 * Initialize logical cores
 * Because Local APIC ID is not contiguous, core id != core num
 */
void CEFICALL bootboot_startcore(void *buf) {
    (void)buf;

    enableNewGDT();

    register CEfiU16 core_num = 0;
    if (lapic_addr) {
        // enable Local APIC
        *((volatile CEfiU32 *)(lapic_addr + 0x0F0)) =
            *((volatile CEfiU32 *)(lapic_addr + 0x0F0)) | 0x100;
        core_num = lapic_ids[*((volatile CEfiU32 *)(lapic_addr + 0x20)) >> 24];
    }
    ap_done = 1;

    // spinlock until BSP finishes (or forever if we got an invalid lapicid,
    // should never happen)
    do {
        __asm__ __volatile__("pause" : : : "memory");
    } while (!bsp_done && core_num != 0xFFFF);

    // enable SSE
    __asm__ __volatile__("movl $0xC0000011, %%eax;"
                         "movq %%rax, %%cr0;"
                         "movq %%cr4, %%rax;"
                         "orw $3 << 8, %%ax;"
                         "mov %%rax, %%cr4"
                         :);

    // set up paging
    __asm__ __volatile__("mov %%rax, %%cr3"
                         :
                         : "a"(globals.level4PageTable)
                         : "memory");

    CEfiU64 stackPointer = globals.highestStackAddress - core_num * PAGESIZE;

    //    __asm__ __volatile__("movq $0xFFFFFFFF, %%rax;"
    //                         "movq %%rax, (%%rdx);"
    //                         "hlt;" ::"d"(globals.frameBufferAddress)
    //                         : "rax");

    // set stack and call _start() in sys/core
    __asm__ __volatile__(
        // get a valid stack for the core we're running on
        "movq %0, %%rsp;"
        // pass control over
        "pushq %1;"

        "retq"
        :
        : "a"(stackPointer), "b"(KERNEL_START)
        : "rsp", "rbp", "rsi", "rcx", "memory");
}

static CEfiU64 ncycles = 1;
CEFICALL void wait(CEfiU64 microseconds) {
    CEfiU32 a;
    CEfiU32 b;
    __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));
    CEfiU64 endtime = (((CEfiU64)b << 32) | a) + microseconds * 200 * ncycles;
    CEfiU64 currTime;
    do {
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));
        currTime = ((CEfiU64)b << 32) | a;
    } while (currTime < endtime);
}

CEfiStatus getSystemConfigTable(CEfiGuid *tableGuid, void **table) {
    CEfiUSize Index;

    for (Index = 0; Index < globals.st->number_of_table_entries; Index++) {
        if (memcmp(tableGuid,
                   &(globals.st->configuration_table[Index].vendor_guid),
                   sizeof(CEfiGuid)) == 0) {
            *table = globals.st->configuration_table[Index].vendor_table;
            return C_EFI_SUCCESS;
        }
    }
    return C_EFI_NOT_FOUND;
}

/**
 * Main EFI application entry point
 */
CEfiStatus efi_main(CEfiHandle handle, CEfiSystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;

    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       C_EFI_BACKGROUND_RED | C_EFI_YELLOW);

    globals.level4PageTable = allocAndZero(1);

    //    EFI_LOADED_IMAGE *loaded_image = NULL;
    //    EFI_GUID lipGuid = LOADED_IMAGE_PROTOCOL;
    //    EFI_GUID RomTableGuid = EFI_PCI_OPTION_ROM_TABLE_GUID;
    //    EFI_PCI_OPTION_ROM_TABLE *RomTable;
    //    EFI_GUID bioGuid = BLOCK_IO_PROTOCOL;
    //    EFI_BLOCK_IO *bio;
    //    EFI_HANDLE *handles = NULL;
    //    CEfiStatus status = EFI_SUCCESS;
    //    EFI_MEMORY_DESCRIPTOR *memory_map = NULL, *mement;
    //    EFI_PARTITION_TABLE_HEADER *gptHdr;
    //    EFI_PARTITION_ENTRY *gptEnt;
    //    EFI_INPUT_KEY key;
    //    CEfiU64 ncycles = 0, currtime, endtime;
    //    CEfiUSize bad_madt = 0;
    //    EFI_GUID SerIoGuid = EFI_SERIAL_IO_PROTOCOL_GUID;
    //    EFI_SERIAL_IO_PROTOCOL *ser = NULL;
    //    CEfiUSize bsp_num = 0, i, j = 0, x, y, handle_size = 0,
    //    memory_map_size = 0,
    //              map_key = 0, desc_size = 0;
    //    CEfiU32 desc_version = 0, a, b;
    //    CEfiU64 lba_s = 0, lba_e = 0, sysptr;
    //    MMapEnt *mmapent, *last = NULL, *sort;
    //    file_t ret = {NULL, 0};

    // unlike BIOS+MultiBoot bootboot, no need to check if we have
    // PAE + MSR + LME, as we're already in long mode.
    __asm__ __volatile__("mov $1, %%eax;"
                         "cpuid;"
                         "shrl $24, %%ebx;"
                         "mov %%rbx,%0"
                         : "=b"(globals.bootstrapProcessorID)
                         :
                         : "eax", "ecx", "edx");

    CEfiU32 a;
    // should be no need to check for RDSTC, available since Pentium, therefore
    // all long mode capable CPUs should have it. But just to be on the safe
    // side
    __asm__ __volatile__("mov $1, %%eax; cpuid;" : "=d"(a) : :);
    if (a & (1 << 4)) {
        // calibrate CPU clock cycles
        CEfiU32 d;
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        CEfiU64 currtime = ((CEfiU64)d << 32) | a;
        globals.st->boot_services->stall(1);
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        ncycles = ((CEfiU64)d << 32) | a;
        ncycles -= currtime;
        ncycles /= 5;
        if (ncycles < 1) {
            ncycles = 1;
        }
    } else {
        error(u"RDSTC not supported\r\n");
    }

    // should be no need to check for RDSTC, available since Pentium, therefore
    // all long mode capable CPUs should have it. But just to be on the safe
    // side
    __asm__ __volatile__("mov $1, %%eax; cpuid;" : "=d"(a) : :);
    if (a & (1 << 4)) {
        // calibrate CPU clock cycles
        CEfiU32 d;
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        CEfiU64 currtime = ((CEfiU64)d << 32) | a;
        globals.st->boot_services->stall(1);
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        ncycles = ((CEfiU64)d << 32) | a;
        ncycles -= currtime;
        ncycles /= 5;
        if (ncycles < 1) {
            ncycles = 1;
        }
    } else {
        // fallback to dummy loops without RDTSC (should never happen)
        ncycles = 0;
    }
#define sleep(n)                                                               \
    do {                                                                       \
        if (ncycles) {                                                         \
            __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));                  \
            endtime = (((CEfiU64)b << 32) | a) + (n) * ncycles;                \
            do {                                                               \
                __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));              \
                currtime = ((CEfiU64)b << 32) | a;                             \
            } while (currtime < endtime);                                      \
        } else                                                                 \
            __asm__ __volatile__(                                              \
                "1: pause; dec %%ecx; or %%ecx, %%ecx; jnz 1b"                 \
                :                                                              \
                : "c"((n) * 1000)                                              \
                : "memory");                                                   \
    } while (0)
#define send_ipi(a, m, v)                                                      \
    do {                                                                       \
        while (*((volatile CEfiU32 *)(lapic_addr + 0x300)) & (1 << 12))        \
            __asm__ __volatile__("pause" : : : "memory");                      \
        *((volatile CEfiU32 *)(lapic_addr + 0x310)) =                          \
            (*((volatile CEfiU32 *)(lapic_addr + 0x310)) & 0x00ffffff) |       \
            ((a) << 24);                                                       \
        *((volatile CEfiU32 *)(lapic_addr + 0x300)) =                          \
            (*((volatile CEfiU32 *)(lapic_addr + 0x300)) & (m)) | (v);         \
    } while (0)

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to read kernel info\r\n");
    DataPartitionFile kernelFile = getKernelInfo();

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to load kernel\r\n");

    AsciString kernelContent = readDiskLbasFromCurrentGlobalImage(
        kernelFile.lbaStart, kernelFile.bytes);

    printNumber((CEfiU64)*kernelContent.buf, 16);
    printNumber((CEfiU64) * (kernelContent.buf + 1), 16);
    printNumber((CEfiU64) * (kernelContent.buf + 2), 16);
    //    CEfiInputKey key;
    //    globals.st->con_out->output_string(globals.st->con_out,
    //                                       u"Press any key to
    //                                       continue...\r\n");
    //    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key)
    //    !=
    //           C_EFI_SUCCESS) {
    //        ;
    //    }

    globals.st->con_out->output_string(
        globals.st->con_out, u"Retrieving Graphics output buffer...\r\n");
    CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;
    CEfiStatus status = globals.st->boot_services->locate_protocol(
        &C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, C_EFI_NULL, (void **)&gop);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    globals.frameBufferAddress = gop->mode->frameBufferBase;

    CEfiU64 sysptr = 0;
    CEfiU64 acpi_ptr = 0;
    CEfiU64 smbi_ptr = 0;
    CEfiU64 mp_ptr = 0;
    getSystemConfigTable(&C_EFI_ACPI_TABLE_GUID, (void *)&sysptr);
    acpi_ptr = sysptr;
    sysptr = 0;
    getSystemConfigTable(&C_EFI_SMBIOS_TABLE_GUID, (void *)&sysptr);
    smbi_ptr = sysptr;
    sysptr = 0;
    getSystemConfigTable(&C_EFI_MPS_TABLE_GUID, (void *)&sysptr);
    mp_ptr = sysptr;

    unsigned char *acpiThing = (unsigned char *)acpi_ptr;
    if (memcmp(acpiThing, "RSDT", 4) && memcmp(acpiThing, "XSDT", 4)) {
        for (CEfiU64 i = 1; i < 256; i++) {
            if (!memcmp(acpiThing + i, "RSD PTR ", 8)) {
                acpiThing += i;
                break;
            }
        }
        // get ACPI system table
        ACPI_RSDPTR *rsd = (ACPI_RSDPTR *)acpiThing;
        if (rsd->xsdt != 0) {
            acpi_ptr = rsd->xsdt;
        } else {
            acpi_ptr = (CEfiU64)((CEfiU32)rsd->rsdt);
        }
    }

    // Symmetric Multi Processing support
    CEfiU8 *ptr = (CEfiU8 *)acpi_ptr, *pe, *data;
    CEfiU64 r;
    CEfiUSize bad_madt = 0;
    CEfiUSize bsp_num = 0;
    CEfiU64 i;
    {
        for (i = 0; i < (int)(sizeof(lapic_ids) / sizeof(lapic_ids[0])); i++) {
            lapic_ids[i] = 0xFFFF;
        }
        if (!nosmp && ptr && (ptr[0] == 'X' || ptr[0] == 'R') &&
            ptr[1] == 'S' && ptr[2] == 'D' && ptr[3] == 'T') {
            pe = ptr;
            ptr += 36;
            // iterate on ACPI table pointers
            for (r = *((CEfiU32 *)(pe + 4)); ptr < pe + r;
                 ptr += pe[0] == 'X' ? 8 : 4) {
                data = (CEfiU8 *)(CEfiU64)(pe[0] == 'X' ? *((CEfiU64 *)ptr)
                                                        : *((CEfiU32 *)ptr));
                if (!memcmp(data, "APIC", 4)) {
                    // found MADT, iterate on its variable length entries
                    lapic_addr = (CEfiU64)(*((CEfiU32 *)(data + 0x24)));
                    for (r = *((CEfiU32 *)(data + 4)), ptr = data + 44, i = 0;
                         ptr < data + r &&
                         i < (int)(sizeof(lapic_ids) / sizeof(lapic_ids[0]));
                         ptr += ptr[1]) {
                        switch (ptr[0]) {
                        case 0: // found Processor Local APIC
                            if ((ptr[4] & 1) && ptr[3] != 0xFF &&
                                lapic_ids[(CEfiISize)ptr[3]] == 0xFFFF)
                                lapic_ids[(CEfiISize)ptr[3]] = (CEfiU16)i++;
                            else
                                bad_madt++;
                            break;
                        case 5:
                            lapic_addr = *((CEfiU64 *)(ptr + 4));
                            break; // found 64 bit Local APIC Address
                        }
                    }
                    if (i) {
                        bsp_num = lapic_ids[globals.bootstrapProcessorID];
                        if (bsp_num == 0xFFFF)
                            bsp_num = 0;
                        else {
                            globals.numberOfCores = i;
                        }
                    }
                    break;
                }
            }
        }
    }

    prepNewGDT();

    if (nosmp || globals.numberOfCores < 2 || !lapic_addr) {
        globals.numberOfCores = 1;
        lapic_addr = 0;
    }

    mapMemoryAt(0, 0, (1ULL << 34)); // First 16 GiB

    mapMemoryAt((CEfiU64)kernelContent.buf, KERNEL_START,
                (CEfiU32)kernelContent.len);

    CEfiPhysicalAddress kernelParams = allocAndZero(1);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, PAGE_SIZE);
    KernelParameters *params = (KernelParameters *)kernelParams;
    params->fb.columns = gop->mode->info->horizontalResolution;
    params->fb.rows = gop->mode->info->verticalResolution;
    params->fb.scanline = gop->mode->info->pixelsPerScanLine;
    params->fb.ptr = gop->mode->frameBufferBase;
    params->fb.size = gop->mode->frameBufferSize;

    mapMemoryAt(gop->mode->frameBufferBase, gop->mode->frameBufferBase,
                gop->mode->frameBufferSize);

    CEfiPhysicalAddress stackEnd = allocAndZero(globals.numberOfCores);
    globals.highestStackAddress = stackEnd + PAGESIZE * globals.numberOfCores;
    mapMemoryAt(stackEnd, KERNEL_STACK_START, globals.numberOfCores * PAGESIZE);

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Number of detected cores: ");
    printNumber(globals.numberOfCores, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Stack space (lowest possible): ");
    printNumber(stackEnd, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    globals.st->con_out->output_string(
        globals.st->con_out, u"Stack space (highest/starting address): ");
    printNumber(globals.highestStackAddress, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    // Get memory map
    bool apmemfree = false;
    MemoryInfo memoryInfo = getMemoryInfo();
    for (CEfiUSize i = 0;
         i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize; i++) {
        CEfiMemoryDescriptor *mement =
            (CEfiMemoryDescriptor *)((CEfiU8 *)memoryInfo.memoryMap +
                                     (i * memoryInfo.descriptorSize));

        if (mement == NULL ||
            (mement->physical_start == 0 && mement->number_of_pages == 0)) {
            break;
        }
        // check if the AP trampoline code's memory is free
        if (mement->type == 7 && mement->physical_start <= (CEfiU64)0x8000 &&
            mement->physical_start + (mement->number_of_pages * PAGESIZE) >
                (CEfiU64)0x8000) {
            apmemfree = true;
        }
    }

    // Assuming we are going to run this on a multicore machine, right???
    if (!apmemfree) {
        error(u"Trampoline code for application processors is not free!\r\n");
    }

    // --- NO PRINT AFTER THIS POINT ---

    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);

    if (C_EFI_ERROR(status)) {
        status = globals.st->boot_services->free_pages(
            (CEfiPhysicalAddress)memoryInfo.memoryMap,
            BYTES_TO_PAGES(memoryInfo.memoryMapSize));
        if (C_EFI_ERROR(status)) {
            error(u"Could not free allocated memory map\r\n");
        }

        memoryInfo = getMemoryInfo();
        status = globals.st->boot_services->exit_boot_services(
            globals.h, memoryInfo.mapKey);
    }
    if (C_EFI_ERROR(status)) {
        error(u"could not exit boot services!\r\n");
    }

    // disable PIC and NMI
    __asm__ __volatile__(
        "movb $0xFF, %%al; outb %%al, $0x21; outb %%al, $0xA1;" // disable
                                                                // PIC
        "inb $0x70, %%al; orb $0x80, %%al; outb %%al, $0x70;"   // disable NMI
        :
        :
        :);

    // start APs
    if (globals.numberOfCores > 1) {
        // copy trampoline and save UEFI's 64 bit system registers for the
        // trampoline code
        __asm__ __volatile__("movq $32, %%rcx; movq %0, %%rsi; movq "
                             "$0x8000, %%rdi; repnz movsq;"
                             "movq %%cr3, %%rax; movq %%rax, 0x80C0;"
                             "movl %%cs, %%eax; movl %%eax, 0x80CC;"
                             "movl %%ds, %%eax; movl %%eax, 0x80D0;"
                             "movq %%rbx, 0x80D8;"
                             "sgdt 0x80E0;"
                             :
                             : "d"((CEfiU64)&ap_trampoline),
                               "b"((CEfiU64)&bootboot_startcore)
                             : "rcx", "rsi", "rdi", "rax", "memory");

        // enable Local APIC
        *((volatile CEfiU32 *)(lapic_addr + 0x0D0)) = (1 << 24);
        *((volatile CEfiU32 *)(lapic_addr + 0x0E0)) = 0xFFFFFFFF;
        *((volatile CEfiU32 *)(lapic_addr + 0x0F0)) =
            *((volatile CEfiU32 *)(lapic_addr + 0x0F0)) | 0x1FF;
        *((volatile CEfiU32 *)(lapic_addr + 0x080)) = 0;
        // make sure we use the correct Local APIC ID for the BSP
        globals.bootstrapProcessorID =
            *((volatile CEfiU32 *)(lapic_addr + 0x20)) >> 24;

        {
            // supports up to 255 cores (lapicid 255 is bcast address),
            // requires x2APIC to have more
            for (i = 0; i < 255; i++) {
                if (i == globals.bootstrapProcessorID || lapic_ids[i] == 0xFFFF)
                    continue;
                *((volatile CEfiU32 *)(lapic_addr + 0x280)) =
                    0; // clear APIC errors
                a = *((volatile CEfiU32 *)(lapic_addr + 0x280));
                send_ipi(i, 0xfff00000, 0x00C500); // trigger INIT IPI
                wait(1);
                send_ipi(i, 0xfff00000, 0x008500); // deassert INIT IPI
            }
            wait(50); // wait 10 msec
            for (i = 0; i < 255; i++) {
                if (i == globals.bootstrapProcessorID || lapic_ids[i] == 0xFFFF)
                    continue;
                ap_done = 0;
                send_ipi(i, 0xfff0f800,
                         0x004608); // trigger SIPI, start at 0800:0000h
                for (a = 250; !ap_done && a > 0; a--)
                    wait(1); // wait for AP with 50 msec timeout
                if (!ap_done) {
                    send_ipi(i, 0xfff0f800, 0x004608);
                    wait(250);
                }
            }
        }
    }
    __asm__ __volatile__("pause" : : : "memory"); // memory barrier

    // release AP spinlock
    bsp_done = 1;
    __asm__ __volatile__("pause" : : : "memory"); // memory barrier
    bootboot_startcore((void *)bsp_num);

    return !C_EFI_SUCCESS;
}
