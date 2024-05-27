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
        LOG(STRING("abc\n"));
        LOG(STRING("efghijklm\n"));
        LOG(STRING("opqr"));

        LOG(STRING("hi 0\n"));
        LOG(STRING("hi 1\n"));
        LOG(STRING("hi 2\n"));
        LOG(STRING("hi 3\n"));
        LOG(STRING("hi 4\n"));
        LOG(STRING("hi 5\n"));
        LOG(STRING("hi 6\n"));
        LOG(STRING("hi 7\n"));
        LOG(STRING("hi 8\n"));
        LOG(STRING("hi 9\n"));
        LOG(STRING("hi 10\n"));
        LOG(STRING("hi 11\n"));
        LOG(STRING("hi 12\n"));
        LOG(STRING("hi 13\n"));
        LOG(STRING("hi 14\n"));
        LOG(STRING("hi 15\n"));
        LOG(STRING("hi 16\n"));
        LOG(STRING("hi 17\n"));
        LOG(STRING("hi 18\n"));
        LOG(STRING("hi 19\n"));
        LOG(STRING("hi 20\n"));
        LOG(STRING("hi 21\n"));
        LOG(STRING("hi 22\n"));
        LOG(STRING("hi 23\n"));
        LOG(STRING("hi 24\n"));
        LOG(STRING("hi 25\n"));
        LOG(STRING("hi 26\n"));
        LOG(STRING("hi 27\n"));
        LOG(STRING("hi 28\n"));
        LOG(STRING("hi 29\n"));
        LOG(STRING("hi 30\n"));
        LOG(STRING("hi 31\n"));
        LOG(STRING("hi 32\n"));
        LOG(STRING("hi 33\n"));
        LOG(STRING("hi 34\n"));
        LOG(STRING("hi 35\n"));
        LOG(STRING("hi 36\n"));
        LOG(STRING("hi 37\n"));
        LOG(STRING("hi 38\n"));
        LOG(STRING("hi 39\n"));
        LOG(STRING("hi 40\n"));
        LOG(STRING("hi 41\n"));
        LOG(STRING("hi 42\n"));
        LOG(STRING("hi 43\n"));
        LOG(STRING("hi 44\n"));
        LOG(STRING("hi 45\n"));
        LOG(STRING("hi 46\n"));
        LOG(STRING("hi 47\n"));
        LOG(STRING("hi 48\n"));
        LOG(STRING("hi 49\n"));
        LOG(STRING("hi 50\n"));
        LOG(STRING("hi 51\n"));
        LOG(STRING("hi 52\n"));
        LOG(STRING("hi 53\n"));
        LOG(STRING("hi 54\n"));
        LOG(STRING("hi 55\n"));
        LOG(STRING("hi 56\n"));
        LOG(STRING("hi 57\n"));
        LOG(STRING("hi 58\n"));
        LOG(STRING("hi 59\n"));

        LOG(STRING("hi 0"));
        LOG(STRING("hi 1"));
        LOG(STRING("hi 2"));
        LOG(STRING("hi 3"));
        LOG(STRING("hi 4"));
        LOG(STRING("hi 5"));
        LOG(STRING("hi 6"));
        LOG(STRING("hi 7"));
        LOG(STRING("hi 8"));
        LOG(STRING("hi 9"));
        LOG(STRING("hi 10"));
        LOG(STRING("hi 11"));
        LOG(STRING("hi 12"));
        LOG(STRING("hi 13"));
        LOG(STRING("hi 14"));
        LOG(STRING("hi 15"));
        LOG(STRING("hi 16"));
        LOG(STRING("hi 17"));
        LOG(STRING("hi 18"));
        LOG(STRING("hi 19"));
        LOG(STRING("hi 20"));
        LOG(STRING("hi 21"));
        LOG(STRING("hi 22"));
        LOG(STRING("hi 23"));
        LOG(STRING("hi 24"));
        LOG(STRING("hi 25"));
        LOG(STRING("hi 26"));
        LOG(STRING("hi 27"));
        LOG(STRING("hi 28"));
        LOG(STRING("hi 29"));
        LOG(STRING("hi 30"));
        LOG(STRING("hi 31"));
        LOG(STRING("hi 32"));
        LOG(STRING("hi 33"));
        LOG(STRING("hi 34"));
        LOG(STRING("hi 35"));
        LOG(STRING("hi 36"));
        LOG(STRING("hi 37"));
        LOG(STRING("hi 38"));
        LOG(STRING("hi 39"));
        LOG(STRING("hi 40"));
        LOG(STRING("hi 41"));
        LOG(STRING("hi 42"));
        LOG(STRING("hi 43"));
        LOG(STRING("hi 44"));
        LOG(STRING("hi 45"));
        LOG(STRING("hi 46"));
        LOG(STRING("hi 47"));
        LOG(STRING("hi 48"));
        LOG(STRING("hi 49"));
        LOG(STRING("hi 50"));
        LOG(STRING("hi 51"));
        LOG(STRING("hi 52"));
        LOG(STRING("hi 53"));
        LOG(STRING("hi 54"));
        LOG(STRING("hi 55"));
        LOG(STRING("hi 56"));
        LOG(STRING("hi 57"));
        LOG(STRING("hi 58"));
        LOG(STRING("hi 59"));
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
