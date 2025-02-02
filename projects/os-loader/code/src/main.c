#include "efi-to-kernel/kernel-parameters.h"  // for KernelParameters
#include "efi-to-kernel/memory/definitions.h" // for STACK_SIZE
#include "efi-to-kernel/memory/descriptor.h"  // for MemoryDescriptor
#include "efi/acpi/rdsp.h"                    // for getRSDP, RSDP...
#include "efi/error.h"
#include "efi/firmware/base.h"               // for PhysicalAddress
#include "efi/firmware/graphics-output.h"    // for GRAPHICS_OUTP...
#include "efi/firmware/simple-text-output.h" // for SimpleTextOut...
#include "efi/firmware/system.h"             // for PhysicalAddress
#include "efi/globals.h"                     // for globals
#include "os-loader/data-reading.h"          // for getKernelInfo
#include "os-loader/gdt.h"                   // for enableNewGDT
#include "os-loader/memory/boot-functions.h" // for mapMemoryAt
#include "os-loader/memory/page-size.h"      // for UEFI_PAGE_SIZE
#include "platform-abstraction/efi.h"
#include "platform-abstraction/log.h"
#include "shared/log.h"
#include "shared/maths/maths.h"             // for CEILING_DIV_V...
#include "shared/text/string.h"             // for CEILING_DIV_V...
#include "shared/types/types.h"             // for U64, U32, USize
#include "x86/memory/definitions/virtual.h" // for PAGE_FRAME_SIZE
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
    // disable PIC and NMI
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

static U32 ecxFeatureInfo;
static U32 edxFeatureInfo;

bool CpuHasFeatures(unsigned int ecx, unsigned int edx) {
    if (ecx != 0) {
        if ((ecxFeatureInfo & ecx) != ecx) {
            return false;
        }
    }

    if (edx != 0) {
        if ((edxFeatureInfo & edx) != edx) {
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

    initArchitecture();

    KFLUSH_AFTER {
        INFO(STRING("CR3 memory location:"));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)globals.level4PageTable, NEWLINE);
    }

    U32 maxSupportCPUID = 0;
    __asm__ __volatile__("mov $0, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "mov %%eax,%0"
                         : "=r"(maxSupportCPUID)
                         :
                         : "eax", "ebx", "ecx", "edx");

    if (maxSupportCPUID < 1) {
        KFLUSH_AFTER {
            ERROR(STRING(
                "CPU does not support CPUID of 1 and above, buy newer CPU.\n"));
        }
        waitKeyThenReset();
    }

    U32 processorVersionInfo = 0;
    __asm__ __volatile__("mov $1, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "mov %%eax,%0;"
                         "mov %%ecx,%1;"
                         "mov %%edx,%2"
                         : "=r"(processorVersionInfo), "=r"(ecxFeatureInfo),
                           "=r"(edxFeatureInfo)
                         :
                         : "eax", "ebx", "ecx", "edx");

    U64 bootstrapProcessorID = 0;
    __asm__ __volatile__("mov $1, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "shrl $24, %%ebx;"
                         "mov %%rbx,%0;"
                         : "=r"(bootstrapProcessorID)
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
        KFLUSH_AFTER { ERROR(STRING("RDSTC not supported, buy newer CPU.\n")); }
        waitKeyThenReset();
    }

    KFLUSH_AFTER { INFO(STRING("Going to read kernel info\n")); }
    DataPartitionFile kernelFile = getKernelInfo();

    KFLUSH_AFTER {
        INFO(STRING("Going to load kernel\n"));
        INFO(STRING("\tbytes: "));
        INFO(kernelFile.bytes, NEWLINE);
        INFO(STRING("\tlba start: "));
        INFO(kernelFile.lbaStart, NEWLINE);
    }

    string kernelContent = readDiskLbasFromCurrentGlobalImage(
        kernelFile.lbaStart, kernelFile.bytes);

    KFLUSH_AFTER {
        INFO(STRING("Read kernel content, at memory location:"));
        INFO(kernelContent.buf, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Attempting to map memory now\n")); }
    mapMemoryAt((U64)kernelContent.buf, KERNEL_CODE_START,
                (U32)kernelContent.len);

    __asm__ __volatile__("cli");

    KFLUSH_AFTER {
        INFO(STRING("Bootstrap processor work before exiting boot services\n"));
    }
    bootstrapProcessorWork();

    KFLUSH_AFTER {
        INFO(STRING(
            "Going to collect necessary info, then exit bootservices\n"));
    }
    GraphicsOutputProtocol *gop = nullptr;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER { ERROR(STRING("Could not locate locate GOP\n")); }
        waitKeyThenReset();
    }

    MemoryInfo memoryInfo = getMemoryInfo();
    for (USize i = 0; i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)memoryInfo.memoryMap +
                                 (i * memoryInfo.descriptorSize));
        mapMemoryAt(desc->physicalStart, desc->physicalStart,
                    desc->numberOfPages * PAGE_FRAME_SIZE);
    }

    mapMemoryAt(gop->mode->frameBufferBase, gop->mode->frameBufferBase,
                gop->mode->frameBufferSize);

    globals.frameBufferAddress = gop->mode->frameBufferBase;

    KFLUSH_AFTER {
        INFO(STRING("The graphics buffer location is at: "));
        INFO(gop->mode->frameBufferBase, NEWLINE);
        INFO(STRING("The graphics buffer size is: "));
        INFO(gop->mode->frameBufferSize, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Allocating space for kernel parameters\n")); }
    PhysicalAddress kernelParams =
        allocAndZero(KERNEL_PARAMS_SIZE / PAGE_FRAME_SIZE);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, KERNEL_PARAMS_SIZE);
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    KernelParameters *params = (KernelParameters *)kernelParams;

    params->level4PageTable = globals.level4PageTable;

    KFLUSH_AFTER { INFO(STRING("Allocating space for stack\n")); }
    // NOTE: It seems we are adding this stuff to the "free" memory in the
    // kernel. We should somehow distinguish between kernel-required memory that
    // was allocated by the efi-application and useless memory.
    PhysicalAddress stackEnd = allocAndZero(STACK_SIZE / PAGE_FRAME_SIZE);
    mapMemoryAt(stackEnd, BOTTOM_STACK, STACK_SIZE);
    PhysicalAddress stackPointer = stackEnd + STACK_SIZE;

    KFLUSH_AFTER {
        INFO(STRING("The stack will go down from: "));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)stackPointer, NEWLINE);
        INFO(STRING("to: "));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)stackEnd, NEWLINE);
    }

    params->fb.columns = gop->mode->info->horizontalResolution;
    params->fb.rows = gop->mode->info->verticalResolution;
    params->fb.scanline = gop->mode->info->pixelsPerScanLine;
    params->fb.ptr = gop->mode->frameBufferBase;
    params->fb.size = gop->mode->frameBufferSize;

    RSDPResult rsdp = getRSDP(globals.st->number_of_table_entries,
                              globals.st->configuration_table);
    if (!rsdp.rsdp) {
        KFLUSH_AFTER { ERROR(STRING("Could not find an RSDP!\n")); }
        waitKeyThenReset();
    }

    if (CpuHasFeatures(0, CPUID_FEAT_EDX_PGE)) {
        KFLUSH_AFTER { INFO(STRING("Enabling GPE\n")); }
        CpuEnableGpe();
    } else {
        KFLUSH_AFTER {
            ERROR(STRING("CPU does not support global memory paging!"));
        }
        waitKeyThenReset();
    }

    // Can we enable FPU?
    if (CpuHasFeatures(0, CPUID_FEAT_EDX_FPU)) {
        KFLUSH_AFTER { INFO(STRING("Enabling FPU\n")); }
        CpuEnableFpu();
    } else {
        KFLUSH_AFTER { ERROR(STRING("CPU does not support FPU!")); }
        waitKeyThenReset();
    }

    // Can we enable SSE?
    if (CpuHasFeatures(0, CPUID_FEAT_EDX_SSE)) {
        KFLUSH_AFTER {
            INFO(STRING(
                "Enabling SSE... even though it doesnt work yet anyway lol\n"));
        }
        CpuEnableSse();
    } else {
        KFLUSH_AFTER { ERROR(STRING("CPU does not support SSE!")); }
        waitKeyThenReset();
    }

    //    // Can we enable xsave? (and maybe avx?)
    //    // We are just checking for xsave and not osxsave currently. I am not
    //    sure
    //    // what the implications would be tbh.
    //    if (CpuHasFeatures(CPUID_FEAT_ECX_XSAVE, 0)) {
    //
    //        KFLUSH_AFTER { INFO(STRING("Enabling XSAVE\n")); }
    //        CpuEnableXSave();
    //
    //        if (CpuHasFeatures(CPUID_FEAT_ECX_AVX, 0)) {
    //            KFLUSH_AFTER { INFO(STRING("Enabling AVX\n")); }
    //            CpuEnableAvx();
    //        } else {
    //            error(u"CPU does not support AVX!");
    //        }
    //    } else {
    //        error(u"CPU does not support XSAVE!");
    //    }

    KFLUSH_AFTER {
        INFO(STRING("Prepared and collected all necessary information to jump "
                    "to the kernel.\nStarting exit boot services process, no "
                    "printing after this!\n"));
    }

    memoryInfo = getMemoryInfo();
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);

    if (EFI_ERROR(status)) {
        status = globals.st->boot_services->free_pages(
            (PhysicalAddress)memoryInfo.memoryMap,
            CEILING_DIV_VALUE(memoryInfo.memoryMapSize, UEFI_PAGE_SIZE));
        if (EFI_ERROR(status)) {
            KFLUSH_AFTER {
                ERROR(STRING("Could not free allocated memory map\r\n"));
            }
            waitKeyThenReset();
        }

        memoryInfo = getMemoryInfo();
        status = globals.st->boot_services->exit_boot_services(
            globals.h, memoryInfo.mapKey);
    }
    if (EFI_ERROR(status)) {
        KFLUSH_AFTER { ERROR(STRING("could not exit boot services!\r\n")); }
        waitKeyThenReset();
    }

    params->memory =
        (KernelMemory){.totalDescriptorSize = memoryInfo.memoryMapSize,
                       .descriptors = memoryInfo.memoryMap,
                       .descriptorSize = memoryInfo.descriptorSize};

    jumpIntoKernel(stackPointer);
    return !SUCCESS;
}
