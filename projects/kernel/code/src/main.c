#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "acpi/madt.h"
#include "acpi/signatures.h"
#include "idt.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "test.h"
#include "util/log.h"
#include "util/types.h"

// void appendDescriptionHeaders(RSDPResult rsdp);

__attribute__((section("kernel-start"))) int kernelmain() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    setupScreen(
        (ScreenDimension){.scanline = kernelParameters->fb.scanline,
                          .size = kernelParameters->fb.size,
                          .width = kernelParameters->fb.columns,
                          .height = kernelParameters->fb.rows,
                          .buffer = (uint32_t *)kernelParameters->fb.ptr});

    setupIDT();

    //    string bssData = STRING_LEN((char *)KERNEL_SPACE_START + 0x8000,
    //    0x5490);
    //
    //    uint64_t nonZeroes = 0;
    //    for (uint64_t i = 0; i < bssData.len; i++) {
    //        char ele = bssData.buf[i];
    //        if (ele != 0) {
    //            nonZeroes++;
    //            LOG(i);
    //            LOG(STRING(" "));
    //        }
    //    }

    //    FLUSH_AFTER {
    //        LOG(STRING("The number of non-zeroes is: "));
    //        LOG(nonZeroes, NEWLINE);
    //    }

    FLUSH_AFTER {
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("BBBBBBBBBBBBBBBBBBBBBBBB\n"));
        LOG(STRING("CCCCCCCCCCCCCCCCCCCCCCCC"));
    }

    FLUSH_AFTER {
        LOG(STRING("DDDDDDDDDDDDDDDDDDDDDDDD\n"));
        LOG(STRING("EEEEEEEEEEEEEEEEEEEEEEEE\n"));
        LOG(STRING("FFFFFFFFFFFFFFFFFFFFFFFF\n"));
    }

    // __asm__ __volatile__("int $3" ::"r"(0));

    // FLUSH_AFTER { appendDescriptionHeaders(kernelParameters->rsdp); }

    while (1) {
        ;
    }
}

// typedef enum { RSDT, XSDT, NUM_DESCRIPTION_TABLES } DescriptionTableVersion;
// static USize entrySizes[NUM_DESCRIPTION_TABLES] = {
//     sizeof(uint32_t),
//     sizeof(uint64_t),
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
//     char **descriptionHeaders = (char **)&sdt->descriptionHeaders;
//     for (uint64_t i = 0; i < sdt->header.length - sizeof(CAcpiSDT);
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
//             for (uint64_t j = 0;
//                  j < madt->madt.header.length - sizeof(ConstantMADT);
//                  j += interruptStructures->totalLength) {
//                 LOG(STRING("Type: "));
//                 LOG(interruptStructures->type);
//                 LOG(STRING("Length: "));
//                 LOG(interruptStructures->totalLength, NEWLINE);
//
//                 interruptStructures = (InterruptControllerStructure
//                                            *)((char *)interruptStructures +
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
//         descriptionHeaders = (char **)((char *)descriptionHeaders +
//         entrySize);
//     }
// }
