#include "acpi/c-acpi-rsdt.h"
#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
#include "efi/c-efi-protocol-simple-text-output.h"
#include "efi/c-efi-system.h"
#include "globals.h"
#include "memory/standard.h"
#include "printing.h"

typedef enum { RSDT, XSDT, NUM_DESCRIPTION_TABLES } DescriptionTableVersion;
static CEfiUSize entrySizes[NUM_DESCRIPTION_TABLES] = {
    sizeof(CEfiU32),
    sizeof(CEfiU64),
};

void printDescriptionHeaders(RSDPResult rsdp) {
    CAcpiSDT *sdt = C_EFI_NULL;
    CEfiUSize entrySize = 0;

    switch (rsdp.revision) {
    case RSDP_REVISION_1: {
        sdt = (CAcpiSDT *)rsdp.rsdp->v1.rsdt_addr;
        entrySize = entrySizes[RSDT];
        break;
    }
    case RSDP_REVISION_2: {
        sdt = (CAcpiSDT *)rsdp.rsdp->v2.xsdt_addr;
        entrySize = entrySizes[XSDT];
        break;
    }
    default: {
        error(u"Unrecognized RSDP revision!\r\n");
    }
    }

    char *descriptionHeaders = (char *)sdt->descriptionHeaders;
    for (CEfiU64 i = 0; i < sdt->header.length - sizeof(CAcpiSDT);
         i += entrySize) {
        CAcpiDescriptionTableHeader *header =
            (CAcpiDescriptionTableHeader *)(descriptionHeaders + i);
        printAsciSize(header->signature, ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN);
        printAsciSize(header->oem_table_id, 6);
        globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    }

    //   printAsciSize(sdt->header.signature,
    //   ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN);
    //   globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    //   int entries = (sdt->header.length - sizeof(sdt->header)) / 8;
    //   for (int i = 0; i < entries; i++) {
    //       CAcpiDescriptionTableHeader *h = sdt->descriptionHeaders[i];
    //       printNumber((CEfiU64)h, 16);
    //       globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //       printNumber((CEfiU64)(&h->signature), 16);
    //       globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //       //        printAsciSize(h->signature,
    //       //        ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN);
    //       //        globals.st->con_out->output_string(globals.st->con_out,
    //       //        u"\r\n");
    //   }

    //    printAsciSize(sdt->header.signature,
    //    ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //
    //    CAcpiDescriptionTableHeader *first =
    //        (CAcpiDescriptionTableHeader *)sdt->descriptionHeaders;
    //    printNumber((CEfiU64)first, 16);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //    printAsciSize(first->signature, 4);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //
    //    CAcpiDescriptionTableHeader *second =
    //        (CAcpiDescriptionTableHeader *)(sdt->descriptionHeaders + 8);
    //    printNumber((CEfiU64)second, 16);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
    //    printAsciSize(second->signature, 4);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    //    printAsciSize((((CEfiU8 *)*(CEfiU64 *)(sdt->descriptionHeaders + 8))),
    //                  ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN);
    //    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
}

// void *acpi_get_table(char *signature, int index) {
//     int cnt = 0;
//
//     CAcpiRSDP *rsdp = acpi_get_rsdp();
//     if (rsdp == C_EFI_NULL) {
//         error(u"Could not find an RSDP for ACPI\r\n");
//     }
//
//     bool use_xsdt = false;
//     if (rsdp->rev >= 2 && rsdp->xsdt_addr) {
//         use_xsdt = true;
//     }
//
//     CAcpiRSDT *rsdt;
//     if (use_xsdt) {
//         rsdt = (CAcpiRSDT *)(CEfiU64)rsdp->xsdt_addr;
//     } else {
//         rsdt = (CAcpiRSDT *)(CEfiU64)rsdp->rsdt_addr;
//     }
//
//     size_t entry_count = (rsdt->header.length - sizeof(CAcpiSDT)) /
//                          (use_xsdt ? XSDT_BYTE_LEN : RSDT_BYTE_LEN);
//
//     for (size_t i = 0; i < entry_count; i++) {
//         CAcpiSDT *ptr;
//         if (use_xsdt) {
//             ptr = (CAcpiSDT *)((CEfiU64 *)rsdt->ptrs_start)[i];
//         } else {
//             ptr = (CAcpiSDT *)(CEfiUSize)((CEfiU32 *)rsdt->ptrs_start)[i];
//         }
//
//         if (!memcmp(ptr->signature, signature, 4) &&
//             !acpi_checksum(ptr, ptr->length) && cnt++ == index) {
//             globals.st->con_out->output_string(globals.st->con_out,
//                                                u"ACPI: Found ");
//             printAsci(signature);
//             globals.st->con_out->output_string(globals.st->con_out, u"at ");
//             printNumber((CEfiU64)ptr, 16);
//             globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
//             return ptr;
//         }
//     }
//
//     globals.st->con_out->output_string(globals.st->con_out,
//                                        u"ACPI: not found: ");
//     printAsci(signature);
//     globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
//     error(u"Not found");
//
//     return C_EFI_NULL;
// }
