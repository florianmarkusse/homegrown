#include "hardware/idt.h"                      // for setupIDT
#include "interoperation/kernel-parameters.h"  // for KernelParameters
#include "interoperation/memory/definitions.h" // for KERNEL_PARAMS_START
#include "interoperation/memory/descriptor.h"
#include "interoperation/types.h" // for U32
#include "memory/management/physical.h"
#include "quickie/hello.h"
#include "text/string.h" // for STRING
#include "util/log.h"    // for LOG, LOG_CHOOSER_IMPL_1, rewind, pro...

// void appendDescriptionHeaders(RSDPResult rsdp);

__attribute__((section("kernel-start"))) int kernelmain() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    setupScreen((ScreenDimension){.scanline = kernelParameters->fb.scanline,
                                  .size = kernelParameters->fb.size,
                                  .width = kernelParameters->fb.columns,
                                  .height = kernelParameters->fb.rows,
                                  .screen = (U32 *)kernelParameters->fb.ptr});
    setupIDT();

    initPhysicalMemoryManager((KernelMemory){
        .totalDescriptorSize = kernelParameters->memory.totalDescriptorSize,
        .descriptors = kernelParameters->memory.descriptors,
        .descriptorSize = kernelParameters->memory.descriptorSize});

    string firstBuffer =
        (string){.buf = (typeof(U8 *))allocContiguousBasePhysicalPages(1),
                 .len = PAGE_SIZE};

    for (U64 i = 0; i < 100; i++) {
        firstBuffer.buf[i] = 'A';
    }
    firstBuffer.buf[100] = '\n';
    firstBuffer.len = 101;

    string secondBuffer =
        (string){.buf = (typeof(U8 *))allocContiguousBasePhysicalPages(1),
                 .len = PAGE_SIZE};

    for (U64 i = 0; i < 100; i++) {
        secondBuffer.buf[i] = 'A';
    }
    secondBuffer.buf[100] = '\n';
    secondBuffer.len = 101;

    printPhysicalMemoryManagerStatus();

    freeBasePhysicalPages((FreeMemory_a){
        .buf = (FreeMemory[]){(FreeMemory){.numberOfPages = 1,
                                           .pageStart = (U64)firstBuffer.buf},
                              (FreeMemory){.numberOfPages = 1,
                                           .pageStart = (U64)secondBuffer.buf}},
        .len = 2,
    });

    FLUSH_AFTER { LOG(getBigNumber(), NEWLINE); }

    printPhysicalMemoryManagerStatus();

    // __asm__ __volatile__("int $3" ::"r"(0));

    // FLUSH_AFTER { appendDescriptionHeaders(kernelParameters->rsdp); }

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
//     CAcpiSDT *sdt = NULL;
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
//         CAcpiDescriptionTableHeader *header = NULL;
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
