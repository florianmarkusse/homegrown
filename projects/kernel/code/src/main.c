#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "util/log.h"
#include "util/types.h"

void appendDescriptionHeaders(RSDPResult rsdp);

__attribute__((ms_abi, section("kernel-start"))) int kernelmain() {
    //    __asm__ __volatile__("movq $0x00FF00FF, %%rax;" // Load the absolute
    //    value
    //                         "movq %%rax, (%%rdx);"     // Store the value at
    //                         the
    //                                                    // address pointed to
    //                                                    by
    //                         "hlt;" ::"d"(*(uint32_t *)KERNEL_PARAMS_START)
    //                         :);

    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;
    setupScreen(
        (ScreenDimension){.scanline = kernelParameters->fb.scanline,
                          .size = kernelParameters->fb.size,
                          .width = kernelParameters->fb.columns,
                          .height = kernelParameters->fb.rows,
                          .buffer = (uint32_t *)kernelParameters->fb.ptr});

    FLUSH_AFTER { LOG(STRING("Operating system starting ...\n")); }

    FLUSH_AFTER { appendDescriptionHeaders(kernelParameters->rsdp); }

    while (1) {
        ;
    }
}

typedef enum { RSDT, XSDT, NUM_DESCRIPTION_TABLES } DescriptionTableVersion;
static USize entrySizes[NUM_DESCRIPTION_TABLES] = {
    sizeof(U32),
    sizeof(U64),
};

void appendDescriptionHeaders(RSDPResult rsdp) {
    CAcpiSDT *sdt = NULL;
    USize entrySize = 0;

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
    }

    char *descriptionHeaders = (char *k)sdt->descriptionHeaders;
    for (U64 i = 0; i < sdt->header.length - sizeof(CAcpiSDT); i += entrySize) {
        CAcpiDescriptionTableHeader *header =
            (CAcpiDescriptionTableHeader *)(descriptionHeaders + i);
        LOG(STRING_LEN(header->signature,
                       ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN));
        LOG(STRING_LEN(header->oem_table_id, 6), NEWLINE);
    }
}
