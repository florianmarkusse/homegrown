#include "abstraction/efi.h"
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
#include "efi/memory.h"
#include "os-loader/data-reading.h"          // for getKernelInfo
#include "os-loader/memory/boot-functions.h" // for mapMemoryAt
#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "abstraction/virtual/map.h"
#include "shared/log.h"
#include "shared/maths/maths.h" // for CEILING_DIV_V...
#include "shared/text/string.h" // for CEILING_DIV_V...
#include "shared/types/types.h" // for U64, U32, USize

Status efi_main(Handle handle, SystemTable *systemtable) {
    globals.h = handle;
    globals.st = systemtable;
    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       BACKGROUND_RED | YELLOW);

    initArchitecture();

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

    mapVirtualRegion(KERNEL_CODE_START,
                     (PagedMemory){.pageStart = (U64)kernelContent.buf,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       kernelContent.len, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);

    KFLUSH_AFTER {
        INFO(STRING(
            "Going to collect necessary info, then exit bootservices\n"));
    }
    GraphicsOutputProtocol *gop = nullptr;
    Status status = globals.st->boot_services->locate_protocol(
        &GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (void **)&gop);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not locate locate GOP\n"));
    }

    MemoryInfo memoryInfo = getMemoryInfo();
    for (USize i = 0; i < memoryInfo.memoryMapSize / memoryInfo.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)memoryInfo.memoryMap +
                                 (i * memoryInfo.descriptorSize));
        mapVirtualRegion(desc->physicalStart,
                         (PagedMemory){.pageStart = (U64)desc->physicalStart,
                                       .numberOfPages = desc->numberOfPages},
                         UEFI_PAGE_SIZE);
    }

    mapVirtualRegion(
        gop->mode->frameBufferBase,
        (PagedMemory){.pageStart = (U64)gop->mode->frameBufferBase,
                      .numberOfPages = CEILING_DIV_VALUE(
                          gop->mode->frameBufferSize, UEFI_PAGE_SIZE)},
        UEFI_PAGE_SIZE);

    globals.frameBufferAddress = gop->mode->frameBufferBase;

    KFLUSH_AFTER {
        INFO(STRING("The graphics buffer location is at: "));
        INFO(gop->mode->frameBufferBase, NEWLINE);
        INFO(STRING("The graphics buffer size is: "));
        INFO(gop->mode->frameBufferSize, NEWLINE);
    }

    KFLUSH_AFTER { INFO(STRING("Allocating space for kernel parameters\n")); }
    PhysicalAddress kernelParams =
        allocAndZero(CEILING_DIV_VALUE(KERNEL_PARAMS_SIZE, UEFI_PAGE_SIZE));

    mapVirtualRegion(KERNEL_PARAMS_START,
                     (PagedMemory){.pageStart = kernelParams,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       KERNEL_PARAMS_SIZE, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);

    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
    KernelParameters *params = (KernelParameters *)kernelParams;

    KFLUSH_AFTER { INFO(STRING("Allocating space for stack\n")); }
    // NOTE: It seems we are adding this stuff to the "free" memory in the
    // kernel. We should somehow distinguish between kernel-required memory that
    // was allocated by the efi-application and useless memory.
    PhysicalAddress stackEnd =
        allocAndZero(CEILING_DIV_VALUE(STACK_SIZE, UEFI_PAGE_SIZE));

    mapVirtualRegion(BOTTOM_STACK,
                     (PagedMemory){.pageStart = stackEnd,
                                   .numberOfPages = CEILING_DIV_VALUE(
                                       STACK_SIZE, UEFI_PAGE_SIZE)},
                     UEFI_PAGE_SIZE);
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
        EXIT_WITH_MESSAGE { ERROR(STRING("Could not find an RSDP!\n")); }
    }

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
        EXIT_WITH_MESSAGE_IF(status) {
            ERROR(STRING("Could not free allocated memory map\r\n"));
        }

        memoryInfo = getMemoryInfo();
        status = globals.st->boot_services->exit_boot_services(
            globals.h, memoryInfo.mapKey);
    }
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("could not exit boot services!\r\n"));
    }

    params->memory =
        (KernelMemory){.totalDescriptorSize = memoryInfo.memoryMapSize,
                       .descriptors = memoryInfo.memoryMap,
                       .descriptorSize = memoryInfo.descriptorSize};

    jumpIntoKernel(stackPointer);
    return !SUCCESS;
}
