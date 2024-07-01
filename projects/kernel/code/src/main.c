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

#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

__attribute__((section("kernel-start"))) int kernelmain() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    setupScreen(
        (ScreenDimension){.scanline = kernelParameters->fb.scanline,
                          .size = kernelParameters->fb.size,
                          .width = kernelParameters->fb.columns,
                          .height = kernelParameters->fb.rows,
                          .screen = (uint32_t *)kernelParameters->fb.ptr});

    setupIDT();

    FLUSH_AFTER { LOG(STRING("\t\t\thi\n")); }

    FLUSH_AFTER {
        LOG(STRING("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n"));
        LOG(STRING("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n"));
        LOG(STRING("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"));
    }

    FLUSH_AFTER {
        LOG(STRING("Dick size:\tlarge\n"));
        LOG(STRING("Height:\t\timpressive\n"));
        LOG(STRING("Height:\t\timpressive\n"));
        LOG(STRING("Money:\t\tyes\n"));
        LOG(STRING("\t\t\tlarge\n"));
    }

    FLUSH_AFTER {
        LOG(STRING(
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
            "BBBBBBBBBBBBBBBBBBBB\t"));
    }

    FLUSH_AFTER { LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n")); }

    FLUSH_AFTER {
        LOG(STRING("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n"));
        LOG(STRING("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\nhi"));
    }

    FLUSH_AFTER {
        LOG(STRING("AAAAAAAAAAAAAAAAAAA"));
        LOG(STRING("BBBBBBBBBBBBBBBBBBB\nCCCCCCCCCCCCCCCCCCCCC"));
    }

    FLUSH_AFTER {
        LOG(STRING("1111111111111111111111111111111\n"));
        LOG(STRING("2222222222222222222222222222222\n"));
        LOG(STRING("3333333333333333333333333333333\n"));
        LOG(STRING("4444444444444444444444444444444\n"));
    }

    FLUSH_AFTER {
        LOG(STRING("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n"));
        LOG(STRING("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n"));
        LOG(STRING("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n"));
        LOG(STRING("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"));
        LOG(STRING("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n"));
        LOG(STRING("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n"));
        LOG(STRING("IIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n"));
        LOG(STRING("JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ\n"));
        LOG(STRING("KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK\n"));
        LOG(STRING("LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL\n"));
        LOG(STRING("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n"));
        LOG(STRING("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN\n"));
        LOG(STRING("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n"));
        LOG(STRING("PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP\n"));
        LOG(STRING("QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ\n"));
        LOG(STRING("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR\n"));
        LOG(STRING("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n"));
        LOG(STRING("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n"));
        LOG(STRING("\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
        LOG(STRING("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"));
    }

    //    rewind(4);
    //    rewind(4);
    //    rewind(4);
    //   rewind(4);

    //    FLUSH_AFTER {
    //        LOG(STRING("5555555555555555555555555555555\n"));
    //        LOG(STRING("6666666666666666666666666666666\n"));
    //        LOG(STRING("7777777777777777777777777777777\n"));
    //        LOG(STRING("8888888888888888888888888888888\n"));
    //    }
    //
    //    prowind(2);
    //    prowind(2);
    //    prowind(3);
    //    prowind(100);
    //
    //    FLUSH_AFTER {
    //        LOG(STRING("9999999999999999999999999999999\n"));
    //        LOG(STRING("9999999999999999999999999999999\n"));
    //        LOG(STRING("9999999999999999999999999999999\n"));
    //        LOG(STRING("9999999999999999999999999999999\n"));
    //    }

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
