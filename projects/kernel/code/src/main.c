#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "acpi/madt.h"
#include "acpi/signatures.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "util/log.h"
#include "util/types.h"

void appendDescriptionHeaders(RSDPResult rsdp);

__attribute__((ms_abi, section("kernel-start"))) int kernelmain() {
    //    __asm__ __volatile__("cli;"
    //                         "movq %%rdx, %%rdi;"
    //                         "movq $0x0F000, %%rax;"
    //                         "add %%rax, %%rdi;"
    //                         "movq $0xFF00FF00FF00FF00, %%rax;"
    //                         "movq $0x4000, %%rcx;"
    //                         "rep stosq;"
    //
    //                         "hlt;" ::"d"((*(uint32_t *)KERNEL_PARAMS_START))
    //                         : "rsp", "rbp", "rax", "rcx");
    //
    //    __asm__ __volatile__("cli; hlt" : : "a"(0xdeadbeef)); // DEBUGGING

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
    sizeof(uint32_t),
    sizeof(uint64_t),
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

    char **descriptionHeaders = (char **)&sdt->descriptionHeaders;
    for (uint64_t i = 0; i < sdt->header.length - sizeof(CAcpiSDT);
         i += entrySize) {
        CAcpiDescriptionTableHeader *header = NULL;
        memcpy(&header, descriptionHeaders, entrySize);

        LOG(STRING_LEN(header->signature, ACPI_DESCRIPTION_TABLE_SIGNATURE_LEN),
            NEWLINE);

        ACPITable tableType =
            ACPITablesToEnum(STRING_LEN(header->signature, 4));

        switch (tableType) {
        case MULTIPLE_APIC_DESCRIPTION_TABLE: {
            MADT *madt = (MADT *)header;

            LOG(STRING("printing the structures of the MADT:"), NEWLINE);

            InterruptControllerStructure *interruptStructures =
                madt->interruptStructures;
            for (uint64_t j = 0;
                 j < madt->madt.header.length - sizeof(ConstantMADT);
                 j += interruptStructures->totalLength) {
                LOG(STRING("Type: "));
                LOG(interruptStructures->type);
                LOG(STRING("Length: "));
                LOG(interruptStructures->totalLength, NEWLINE);

                interruptStructures = (InterruptControllerStructure
                                           *)((char *)interruptStructures +
                                              interruptStructures->totalLength);
            }

            break;
        }
        default: {
            LOG(STRING("Did not implement anything for this yet"), NEWLINE);
        }
        }

        descriptionHeaders = (char **)((char *)descriptionHeaders + entrySize);
    }
}
