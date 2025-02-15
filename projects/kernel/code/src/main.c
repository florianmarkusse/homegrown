#include "efi-to-kernel/kernel-parameters.h"  // for KernelParameters
#include "efi-to-kernel/memory/definitions.h" // for KERNEL_PARAMS_START
#include "kernel/log/init.h"
#include "peripheral/screen/screen.h"
#include "platform-abstraction/interrupts.h" // for setupIDT
#include "platform-abstraction/log.h" // for LOG, LOG_CHOOSER_IMPL_1, rewind, pro...
#include "platform-abstraction/memory/management/init.h"
#include "platform-abstraction/memory/management/policy.h"
#include "platform-abstraction/memory/management/status.h"
#include "platform-abstraction/physical/status.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/sizes.h"
#include "shared/text/string.h" // for STRING
#include "shared/types/types.h" // for U32

// void appendDescriptionHeaders(RSDPResult rsdp);

static constexpr auto INIT_MEMORY = (64 * MiB);

__attribute__((section("kernel-start"))) int kernelmain() {
    initIDT();

    KernelParameters *kernelParameters =
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        (KernelParameters *)KERNEL_PARAMS_START;

    KernelMemory kernelMemory = {
        .totalDescriptorSize = kernelParameters->memory.totalDescriptorSize,
        .descriptors = kernelParameters->memory.descriptors,
        .descriptorSize = kernelParameters->memory.descriptorSize};
    initMemoryManager(kernelMemory, kernelParameters->rootPageTable);

    void *initMemory = allocAndMap(INIT_MEMORY);
    Arena arena = (Arena){.curFree = initMemory,
                          .beg = initMemory,
                          .end = initMemory + INIT_MEMORY};
    void *jumper[5];
    if (__builtin_setjmp(jumper)) {
        KFLUSH_AFTER { KLOG(STRING("Ran out of init memory capacity\n")); }
        while (1) {
            ;
        }
    }
    arena.jmp_buf = jumper;

    initLogger(&arena);
    initScreen((ScreenDimension){.scanline = kernelParameters->fb.scanline,
                                 .size = kernelParameters->fb.size,
                                 .width = kernelParameters->fb.columns,
                                 .height = kernelParameters->fb.rows,
                                 /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
                                 .screen = (U32 *)kernelParameters->fb.ptr},
               &arena);
    freeMapped((U64)arena.curFree, (U64)(arena.end - arena.curFree));

    KFLUSH_AFTER { KLOG(STRING("HELLOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n")); }

    KFLUSH_AFTER {
        //
        appendMemoryManagementStatus();
    }

    while (1) {
        ;
    }
}

// typedef enum { RSDT, XSDT, NUM_DESCRIPTION_TABLES } DescriptionTableVersion;
// static USize entrySizes[NUM_DESCRIPTION_TABLES] = {
//     sizeof(U32),
//     sizeof(U64),
// };
//
// void appendDescriptionHeaders(RSDPResult rsdp) {
//     CAcpiSDT *sdt = nullptr;
//     USize entrySize = 0;
//
//     switch (rsdp.revision) {
//     case RSDP_REVISION_1: {
//         sdt = (CAcpiSDT *)rsdp.rsdp->v1.rsdt_addr;
//         entrySize = entrySizes[RSDT];
//         break;
//     }
//     case RSDP_REVISION_2: {
//         sdt = (CAcpiSDT *)rsdp.rsdp->v2.xsdt_addr;
//         entrySize = entrySizes[XSDT];
//         break;
//     }
//     }
//
//     I8 **descriptionHeaders = (I8 **)&sdt->descriptionHeaders;
//     for (U64 i = 0; i < sdt->header.length - sizeof(CAcpiSDT);
//          i += entrySize) {
//         CAcpiDescriptionTableHeader *header = nullptr;
//         memcpy(&header, descriptionHeaders, entrySize);
//
//         LOG(STRING_LEN(header->signature,
//         ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN),
//             NEWLINE);
//
//         ACPITable tableType =
//             ACPITablesToEnum(STRING_LEN(header->signature, 4));
//
//         switch (tableType) {
//         case MULTIPLE_APIC_DESCRIPTION_TABLE: {
//             MADT *madt = (MADT *)header;
//
//             LOG(STRING("printing the structures of the MADT:"), NEWLINE);
//
//             InterruptControllerStructure *interruptStructures =
//                 madt->interruptStructures;
//             for (U64 j = 0;
//                  j < madt->madt.header.length - sizeof(ConstantMADT);
//                  j += interruptStructures->totalLength) {
//                 LOG(STRING("Type: "));
//                 LOG(interruptStructures->type);
//                 LOG(STRING("Length: "));
//                 LOG(interruptStructures->totalLength, NEWLINE);
//
//                 interruptStructures = (InterruptControllerStructure
//                                            *)((I8 *)interruptStructures +
//                                               interruptStructures->totalLength);
//             }
//
//             break;
//         }
//         default: {
//             LOG(STRING("Did not implement anything for this yet"), NEWLINE);
//         }
//         }
//
//         descriptionHeaders = (I8 **)((I8 *)descriptionHeaders +
//         entrySize);
//     }
// }
