#include "acpi/c-acpi-rdsp.h"                      // for getRSDP, RSDPResult
#include "data-reading.h"                          // for getKernelInfo
#include "efi/c-efi-base.h"                        // for PhysicalAddress
#include "efi/c-efi-protocol-graphics-output.h"    // for GRAPHICS_OUTPUT_P...
#include "efi/c-efi-protocol-simple-text-output.h" // for SimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for MemoryDescriptor
#include "gdt.h"                                   // for enableNewGDT, pre...
#include "globals.h"                               // for globals
#include "kernel-parameters.h"                     // for KernelParameters
#include "memory/boot-functions.h"                 // for mapMemoryAt, allo...
#include "memory/definitions.h"                    // for PAGE_SIZE, STACK_...
#include "printing.h"                              // for error, printNumber
#include "string.h"                                // for AsciString
#include "types.h"                                 // for U64, U32, NULL

// static U8 in_exc = 0;

// // Not sure what we are doing when we encounter an exception tbh.
// void fw_exc(U8 excno, U64 exccode, U64 rip, U64 rsp) {
//     U64 cr2, cr3;
//     if (!in_exc) {
//         in_exc++;
//         __asm__ __volatile__("movq %%cr2, %%rax;movq %%cr3, %%rbx;"
//                              : "=a"(cr2), "=b"(cr3)::);
//         error(u"Ran into the first exception?\r\n");
//     }
//     error(u"Ran into the second exception????\r\n");
//     __asm__ __volatile__("1: cli; hlt; jmp 1b");
// }

#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

void flo_printToScreen(PhysicalAddress graphics, U32 color) {
    for (U64 x = 0; x < 100; x++) {
        ((U32 *)graphics)[x] = color;
    }
}

// EFICALL void nonBootstrapProcessor(void *buffer) {
//     __asm__ __volatile__("cli;"
//                          "hlt;"
//                          // TODO: switch to monitor mwait?
//                          //       "monitor;"
//                          //       "mwait"
//                          :
//                          :
//                          :);
// }

// struct interrupt_frame;
//
// __attribute__((interrupt)) void
// interrupt_handler(struct interrupt_frame *frame) {
//     *(U32 *)globals.frameBufferAddress = 0xFFFFFFFF;
// }

// static inline void outb(U16 port, U8 value) {
//     asm volatile("outb %%al, %1" : : "a"(value), "Nd"(port) : "memory");
// }

EFICALL void bootstrapProcessorWork() {
    // dissable PIC and NMI
    __asm__ __volatile__("movb $0xFF, %%al;"
                         "outb %%al, $0x21;"
                         "outb %%al, $0xA1;" // disable PIC
                         "inb $0x70, %%al;"
                         "orb $0x80, %%al;"
                         "outb %%al, $0x70;" // disable NMI
                         :
                         :
                         : "eax", "memory");

    //    // Flush possible old PIC IRQs
    //    for (int i = 0; i < 16; i++) {
    //        if (i >= 8) {
    //            outb(0xa0, 0x20);
    //        }
    //
    //        outb(0x20, 0x20);
    //    }
    //
    //    *((volatile U32 *)(APIC_ERROR_STATUS_REGISTER)) =
    //        0; // clear APIC errors
    //
    //    *((volatile U16 *)(APIC_SIV_REGISTER)) =
    //        *((volatile U16 *)(APIC_SIV_REGISTER)) |
    //        APIC_SIV_ENABLE_LOCAL;

    //   APIC_IPI_ICR_SET_LOW(APIC_INIT_IPI)
    //   globals.st->boot_services->stall(10000);
    //   APIC_IPI_ICR_SET_LOW(APIC_START_UP_IPI)
    //   globals.st->boot_services->stall(200);
    //   APIC_IPI_ICR_SET_LOW(APIC_START_UP_IPI)

    prepNewGDT();

    globals.st->boot_services->stall(100000);

    __asm__ __volatile__("pause" : : : "memory"); // memory barrier
}

enum CpuFeatures {
    // Features contained in ECX register
    CPUID_FEAT_ECX_SSE3 = 1 << 0,
    CPUID_FEAT_ECX_PCLMUL = 1 << 1,
    CPUID_FEAT_ECX_DTES64 = 1 << 2,
    CPUID_FEAT_ECX_MONITOR = 1 << 3,
    CPUID_FEAT_ECX_DS_CPL = 1 << 4,
    CPUID_FEAT_ECX_VMX = 1 << 5,
    CPUID_FEAT_ECX_SMX = 1 << 6,
    CPUID_FEAT_ECX_EST = 1 << 7,
    CPUID_FEAT_ECX_TM2 = 1 << 8,
    CPUID_FEAT_ECX_SSSE3 = 1 << 9,
    CPUID_FEAT_ECX_CID = 1 << 10,
    CPUID_FEAT_ECX_FMA = 1 << 12,
    CPUID_FEAT_ECX_CX16 = 1 << 13,
    CPUID_FEAT_ECX_ETPRD = 1 << 14,
    CPUID_FEAT_ECX_PDCM = 1 << 15,
    CPUID_FEAT_ECX_DCA = 1 << 18,
    CPUID_FEAT_ECX_SSE4_1 = 1 << 19,
    CPUID_FEAT_ECX_SSE4_2 = 1 << 20,
    CPUID_FEAT_ECX_x2APIC = 1 << 21,
    CPUID_FEAT_ECX_MOVBE = 1 << 22,
    CPUID_FEAT_ECX_POPCNT = 1 << 23,
    CPUID_FEAT_ECX_AES = 1 << 25,
    CPUID_FEAT_ECX_XSAVE = 1 << 26,
    CPUID_FEAT_ECX_OSXSAVE = 1 << 27,
    CPUID_FEAT_ECX_AVX = 1 << 28,
    CPUID_FEAT_ECX_F16C = 1 << 29,
    CPUID_FEAT_ECX_RDRAND = 1 << 30,

    // Features contained in EDX register
    CPUID_FEAT_EDX_FPU = 1 << 0,
    CPUID_FEAT_EDX_VME = 1 << 1,
    CPUID_FEAT_EDX_DE = 1 << 2,
    CPUID_FEAT_EDX_PSE = 1 << 3,
    CPUID_FEAT_EDX_TSC = 1 << 4,
    CPUID_FEAT_EDX_MSR = 1 << 5,
    CPUID_FEAT_EDX_PAE = 1 << 6,
    CPUID_FEAT_EDX_MCE = 1 << 7,
    CPUID_FEAT_EDX_CX8 = 1 << 8,
    CPUID_FEAT_EDX_APIC = 1 << 9,
    CPUID_FEAT_EDX_SEP = 1 << 11,
    CPUID_FEAT_EDX_MTRR = 1 << 12,
    CPUID_FEAT_EDX_PGE = 1 << 13,
    CPUID_FEAT_EDX_MCA = 1 << 14,
    CPUID_FEAT_EDX_CMOV = 1 << 15,
    CPUID_FEAT_EDX_PAT = 1 << 16,
    CPUID_FEAT_EDX_PSE36 = 1 << 17,
    CPUID_FEAT_EDX_PSN = 1 << 18,
    CPUID_FEAT_EDX_CLF = 1 << 19,
    CPUID_FEAT_EDX_DTES = 1 << 21,
    CPUID_FEAT_EDX_ACPI = 1 << 22,
    CPUID_FEAT_EDX_MMX = 1 << 23,
    CPUID_FEAT_EDX_FXSR = 1 << 24,
    CPUID_FEAT_EDX_SSE = 1 << 25,
    CPUID_FEAT_EDX_SSE2 = 1 << 26,
    CPUID_FEAT_EDX_SS = 1 << 27,
    CPUID_FEAT_EDX_HTT = 1 << 28,
    CPUID_FEAT_EDX_TM1 = 1 << 29,
    CPUID_FEAT_EDX_IA64 = 1 << 30,
    CPUID_FEAT_EDX_PBE = 1 << 31
};

extern void CpuEnableXSave(void);
extern void CpuEnableAvx(void);
extern void CpuEnableSse(void);
extern void CpuEnableGpe(void);
extern void CpuEnableFpu(void);

bool CpuHasFeatures(unsigned int ecx, unsigned int edx) {
    // Check ECX features @todo multiple cpus
    if (ecx != 0) {
        if ((globals.ecxFeatureInfo & ecx) != ecx) {
            return false;
        }
    }

    // Check EDX features @todo multiple cpus
    if (edx != 0) {
        if ((globals.edxFeatureInfo & edx) != edx) {
            return false;
        }
    }

    return true;
}

EFICALL void jumpIntoKernel(PhysicalAddress stackPointer) {
    enableNewGDT();

    __asm__ __volatile__("mov %%rax, %%cr3"
                         :
                         : "a"(globals.level4PageTable)
                         : "memory");

    __asm__ __volatile__(
        //        "movq $0xFFFFFFFF, %%rax;"  "movq %%rax, (%%rdx);"      "hlt;"
        "cli;"

        //      "movq %%rdx, %%rdi;"
        //      "movq $0xFFFFFFFFFFFFFF, %%rax;"
        //      "movq $0x4000, %%rcx;"
        //      "rep stosq;"

        // "call *%1"

        "movq %0, %%rsp;"
        "movq %%rsp, %%rbp;"

        "cld;"

        "pushq %1;"
        "retq"
        :
        : "r"(stackPointer), "r"(KERNEL_CODE_START)
        // "d"(globals.frameBufferAddress)
        : "memory");

    __builtin_unreachable();

    //
    //    // Set PAT as:
    //    // PAT0 -> WB  (06)
    //    // PAT1 -> WT  (04)
    //    // PAT2 -> UC- (07)
    //    // PAT3 -> UC  (00)
    //    // PAT4 -> WP  (05)
    //    // PAT5 -> WC  (01)
    //    U64 pat = (U64)0x010500070406;
    //    wrmsr(0x277, pat);
}

static U64 ncycles = 1;
EFICALL void wait(U64 microseconds) {
    U32 a;
    U32 b;
    __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));
    U64 endtime = (((U64)b << 32) | a) + microseconds * 200 * ncycles;
    U64 currTime;
    do {
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(b));
        currTime = ((U64)b << 32) | a;
    } while (currTime < endtime);
}

EFICALL Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;

    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_RED | YELLOW);

    globals.level4PageTable = allocAndZero(1);

    __asm__ __volatile__("mov $0, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "mov %%eax,%0"
                         : "=r"(globals.maxSupportCPUID)
                         :
                         : "eax", "ebx", "ecx", "edx");

    if (globals.maxSupportCPUID < 1) {
        error(u"CPU does not support cpu id of 1 and above, "
              u"buy newer cpu.\r\n");
    }

    __asm__ __volatile__("mov $1, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "mov %%eax,%0;"
                         "mov %%ecx,%1;"
                         "mov %%edx,%2"
                         : "=r"(globals.processorVersionInfo),
                           "=r"(globals.ecxFeatureInfo),
                           "=r"(globals.edxFeatureInfo)
                         :
                         : "eax", "ebx", "ecx", "edx");

    __asm__ __volatile__("mov $1, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "shrl $24, %%ebx;"
                         "mov %%rbx,%0;"
                         : "=r"(globals.bootstrapProcessorID)
                         :
                         : "eax", "ebx", "ecx", "edx");

    U32 a;
    // should be no need to check for RDSTC, available since Pentium, therefore
    // all long mode capable CPUs should have it. But just to be on the safe
    // side
    __asm__ __volatile__("mov $1, %%eax; cpuid;" : "=d"(a) : :);
    if (a & (1 << 4)) {
        // calibrate CPU clock cycles
        U32 d;
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        U64 currtime = ((U64)d << 32) | a;
        globals.st->boot_services->stall(1);
        __asm__ __volatile__("rdtsc" : "=a"(a), "=d"(d));
        ncycles = ((U64)d << 32) | a;
        ncycles -= currtime;
        ncycles /= 5;
        if (ncycles < 1) {
            ncycles = 1;
        }
    } else {
        error(u"RDSTC not supported\r\n");
    }

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to read kernel info\r\n");
    DataPartitionFile kernelFile = getKernelInfo();

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to load kernel\r\n");
    AsciString kernelContent = readDiskLbasFromCurrentGlobalImage(
        kernelFile.lbaStart, kernelFile.bytes);

    globals.st->con_out->output_string(
        globals.st->con_out, u"Read kernel content, at memory location:");
    printNumber((USize)kernelContent.buf, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Attempting to map memory now...\r\n");
    mapMemoryAt((U64)kernelContent.buf, KERNEL_CODE_START,
                (U32)kernelContent.len);

    __asm__ __volatile__("cli");

    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Bootstrap processor work before exiting boot services...\r\n");
    bootstrapProcessorWork();

    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Going to collect necessary info, then exit bootservices...\r\n");
    GraphicsOutputProtocol *gop = NULL;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, NULL, (void **)&gop);
    if (ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    MemoryInfo memoryInfo = getMemoryInfo();
    for (USize i = 0; i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)memoryInfo.memoryMap +
                                 (i * memoryInfo.descriptorSize));
        mapMemoryAt(desc->physical_start, desc->physical_start,
                    desc->number_of_pages * PAGE_SIZE);
    }

    mapMemoryAt(gop->mode->frameBufferBase, gop->mode->frameBufferBase,
                gop->mode->frameBufferSize);

    globals.frameBufferAddress = gop->mode->frameBufferBase;
    globals.st->con_out->output_string(globals.st->con_out,
                                       u"The graphics buffer location is at ");
    printNumber(gop->mode->frameBufferBase, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    globals.st->con_out->output_string(
        globals.st->con_out, u"Creating space for kernel parameters...\r\n");
    PhysicalAddress kernelParams = allocAndZero(KERNEL_PARAMS_SIZE / PAGE_SIZE);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, KERNEL_PARAMS_SIZE);
    KernelParameters *params = (KernelParameters *)kernelParams;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Creating space for stack...\r\n");
    PhysicalAddress stackEnd = allocAndZero(STACK_SIZE / PAGE_SIZE);
    mapMemoryAt(stackEnd, BOTTOM_STACK, STACK_SIZE);
    PhysicalAddress stackPointer = stackEnd + STACK_SIZE;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"The stack will go down from ");
    printNumber(stackPointer, 16);
    globals.st->con_out->output_string(globals.st->con_out, u" to ");
    printNumber(stackEnd, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    params->fb.columns = gop->mode->info->horizontalResolution;
    params->fb.rows = gop->mode->info->verticalResolution;
    params->fb.scanline = gop->mode->info->pixelsPerScanLine;
    params->fb.ptr = gop->mode->frameBufferBase;
    params->fb.size = gop->mode->frameBufferSize;

    RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,
                              globals.st->configuration_table);
    if (!rsdp.rsdp) {
        error(u"Could not find an RSDP!\r\n");
    }

    if (CpuHasFeatures(0, CPUID_FEAT_EDX_PGE)) {
        globals.st->con_out->output_string(globals.st->con_out,
                                           u"Enabling GPE...\r\n");
        CpuEnableGpe();
    } else {
        error(u"CPU does not support global memory paging!");
    }

    // Can we enable FPU?
    if (CpuHasFeatures(0, CPUID_FEAT_EDX_FPU)) {
        globals.st->con_out->output_string(globals.st->con_out,
                                           u"Enabling FPU...\r\n");
        CpuEnableFpu();
    } else {
        error(u"CPU does not support FPU!");
    }

    // Can we enable SSE?
    if (CpuHasFeatures(0, CPUID_FEAT_EDX_SSE)) {
        globals.st->con_out->output_string(globals.st->con_out,
                                           u"Enabling SSE...\r\n");
        CpuEnableSse();
    } else {
        error(u"CPU does not support SSE!");
    }

    //    // Can we enable xsave? (and maybe avx?)
    //    // We are just checking for xsave and not osxsave currently. I am not
    //    sure
    //    // what the implications would be tbh.
    //    if (CpuHasFeatures(CPUID_FEAT_ECX_XSAVE, 0)) {
    //        globals.st->con_out->output_string(globals.st->con_out,
    //                                           u"Enabling XSAVE...\r\n");
    //        CpuEnableXSave();
    //
    //        if (CpuHasFeatures(CPUID_FEAT_ECX_AVX, 0)) {
    //            globals.st->con_out->output_string(globals.st->con_out,
    //                                               u"Enabling AVX...\r\n");
    //            CpuEnableAvx();
    //        } else {
    //            error(u"CPU does not support AVX!");
    //        }
    //    } else {
    //        error(u"CPU does not support XSAVE!");
    //    }

    globals.st->con_out->output_string(
        globals.st->con_out, u"Prepared and collected all necessary "
                             u"information to jump to the kernel.\r\n");
    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Starting exit boot services process, no printing after this!\r\n");

    memoryInfo = getMemoryInfo();
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);

    if (ERROR(status)) {
        status = globals.st->boot_services->free_pages(
            (PhysicalAddress)memoryInfo.memoryMap,
            BYTES_TO_PAGES(memoryInfo.memoryMapSize));
        if (ERROR(status)) {
            error(u"Could not free allocated memory map\r\n");
        }

        memoryInfo = getMemoryInfo();
        status = globals.st->boot_services->exit_boot_services(
            globals.h, memoryInfo.mapKey);
    }
    if (ERROR(status)) {
        error(u"could not exit boot services!\r\n");
    }

    jumpIntoKernel(stackPointer);
    return !SUCCESS;
}
