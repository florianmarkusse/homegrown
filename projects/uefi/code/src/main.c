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

#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

void flo_printToScreen(CEfiPhysicalAddress graphics, CEfiU32 color) {
    for (CEfiU64 x = 0; x < 100; x++) {
        ((CEfiU32 *)graphics)[x] = color;
    }
}

CEFICALL void nonBootstrapProcessor(void *buffer) {
    __asm__ __volatile__("cli;"
                         "hlt;"
                         // TODO: switch to monitor mwait?
                         //       "monitor;"
                         //       "mwait"
                         :
                         :
                         :);
}

struct interrupt_frame;

__attribute__((interrupt)) void
interrupt_handler(struct interrupt_frame *frame) {
    *(CEfiU32 *)globals.frameBufferAddress = 0xFFFFFFFF;
}

static inline void outb(CEfiU16 port, CEfiU8 value) {
    asm volatile("outb %%al, %1" : : "a"(value), "Nd"(port) : "memory");
}

CEFICALL void bootstrapProcessorWork() {
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
    //    *((volatile CEfiU32 *)(APIC_ERROR_STATUS_REGISTER)) =
    //        0; // clear APIC errors
    //
    //    *((volatile CEfiU16 *)(APIC_SIV_REGISTER)) =
    //        *((volatile CEfiU16 *)(APIC_SIV_REGISTER)) |
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

CEFICALL void jumpIntoKernel(CEfiPhysicalAddress stackPointer) {
    enableNewGDT();

    // enable SSE
    __asm__ __volatile__("movl $0xC0000011, %%eax;"
                         "movq %%rax, %%cr0;"
                         "movq %%cr4, %%rax;"
                         "orw $3 << 8, %%ax;"
                         "mov %%rax, %%cr4" ::
                             : "eax");

    __asm__ __volatile__("mov %%rax, %%cr3"
                         :
                         : "a"(globals.level4PageTable)
                         : "memory");

    typedef void (*Entry)(void);
    Entry entry = (Entry)KERNEL_START;

    __asm__ __volatile__(
        "movq %0, %%rsp;"
        "movq %%rsp, %%rbp;"
        //        "movq $0xFFFFFFFF, %%rax;"  "movq %%rax, (%%rdx);"      "hlt;"

        "movq %%rdx, %%rdi;"
        "movq $0xFFFFFFFFFFFFFF, %%rax;"
        "movq $0x4000, %%rcx;"
        "rep stosq;"

        // "call *%1"
        "pushq %1;"
        "retq"
        //
        //

        :
        : "r"(stackPointer), "r"(KERNEL_START),
          "d"(*(CEfiU32 *)KERNEL_PARAMS_START)
        //,  "d"(globals.frameBufferAddress)
        : "rsp", "rbp", "rax", "rsi", "rcx", "memory");

    __builtin_unreachable();

    //
    //    // Set PAT as:
    //    // PAT0 -> WB  (06)
    //    // PAT1 -> WT  (04)
    //    // PAT2 -> UC- (07)
    //    // PAT3 -> UC  (00)
    //    // PAT4 -> WP  (05)
    //    // PAT5 -> WC  (01)
    //    CEfiU64 pat = (CEfiU64)0x010500070406;
    //    wrmsr(0x277, pat);
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

CEFICALL CEfiStatus efi_main(CEfiHandle handle, CEfiSystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;

    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       C_EFI_BACKGROUND_RED | C_EFI_YELLOW);

    globals.level4PageTable = allocAndZero(1);

    __asm__ __volatile__("mov $0, %%eax;"
                         "mov $0, %%ecx;"
                         "cpuid;"
                         "mov %%eax,%0"
                         : "=r"(globals.maxSupportCPUID)
                         :
                         : "eax", "ebx", "ecx", "edx");

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
    CEfiInputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           C_EFI_SUCCESS) {
        ;
    }

    globals.st->con_out->output_string(
        globals.st->con_out, u"Read kernel content, at memory location:");
    printNumber((CEfiUSize)kernelContent.buf, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Attempting to map memory now...\r\n");
    mapMemoryAt((CEfiU64)kernelContent.buf, KERNEL_START,
                (CEfiU32)kernelContent.len);

    __asm__ __volatile__("cli");

    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Bootstrap processor work before exiting boot services...\r\n");
    bootstrapProcessorWork();

    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Going to collect necessary info, then exit bootservices...\r\n");
    CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;
    CEfiStatus status = globals.st->boot_services->locate_protocol(
        &C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, C_EFI_NULL, (void **)&gop);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    MemoryInfo memoryInfo = getMemoryInfo();
    for (CEfiUSize i = 0;
         i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize; i++) {
        CEfiMemoryDescriptor *desc =
            (CEfiMemoryDescriptor *)((CEfiU8 *)memoryInfo.memoryMap +
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

    CEfiPhysicalAddress kernelParams = allocAndZero(1);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, PAGE_SIZE);
    KernelParameters *params = (KernelParameters *)kernelParams;

    CEfiPhysicalAddress stackEnd = allocAndZero(STACK_SIZE / PAGE_SIZE);
    mapMemoryAt(stackEnd, BOTTOM_STACK, STACK_SIZE);
    CEfiPhysicalAddress stackPointer = stackEnd + STACK_SIZE - 1;

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
    params->rsdp = rsdp;

    globals.st->con_out->output_string(
        globals.st->con_out, u"Prepared and collected all necessary "
                             u"information to jump to the kernel.\r\n");
    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Starting exit boot services process, no printing after this!\r\n");

    memoryInfo = getMemoryInfo();
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

    jumpIntoKernel(stackPointer);
    return !C_EFI_SUCCESS;
}
