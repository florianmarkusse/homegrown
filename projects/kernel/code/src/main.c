#include "idt.h"               // for setupIDT
#include "kernel-parameters.h" // for KernelParameters
#include "memory-management.h"
#include "memory/definitions.h" // for KERNEL_PARAMS_START
#include "types.h"              // for U32
#include "util/log.h"           // for LOG, LOG_CHOOSER_IMPL_1, rewind, pro...
#include "util/text/string.h"   // for STRING

// void appendDescriptionHeaders(RSDPResult rsdp);

#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

__attribute__((section("kernel-start"))) int kernelmain() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    setupScreen((ScreenDimension){.scanline = kernelParameters->fb.scanline,
                                  .size = kernelParameters->fb.size,
                                  .width = kernelParameters->fb.columns,
                                  .height = kernelParameters->fb.rows,
                                  .screen = (U32 *)kernelParameters->fb.ptr});
    setupIDT();

    KernelMemory kernelMemory = (KernelMemory){
        .totalDescriptorSize = kernelParameters->memory.totalDescriptorSize,
        .descriptors = kernelParameters->memory.descriptors,
        .descriptorSize = kernelParameters->memory.descriptorSize};

    for (U64 i = 0; i < kernelMemory.descriptorSize * 10;
         i += kernelMemory.descriptorSize) {
        MemoryDescriptor *descriptor =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors + i);

        FLUSH_AFTER {
            LOG(STRING("PAddr:\t"));
            LOG((void *)descriptor->physical_start);
            LOG(STRING("\tVAddr:\t"));
            LOG(descriptor->virtual_start);
            LOG(STRING("\tNumber of pages:\t"));
            LOG(descriptor->number_of_pages, NEWLINE);
        }
    }

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
