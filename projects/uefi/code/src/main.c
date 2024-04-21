#include "acpi/c-acpi-madt.h"
#include "acpi/c-acpi-rdsp.h"
#include "acpi/c-acpi-rsdt.h"
#include "data-reading.h"
#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
#include "efi/c-efi-protocol-acpi.h"
#include "efi/c-efi-protocol-block-io.h"
#include "efi/c-efi-protocol-disk-io.h"
#include "efi/c-efi-protocol-graphics-output.h"
#include "efi/c-efi-protocol-loaded-image.h"
#include "efi/c-efi-protocol-simple-file-system.h"
#include "efi/c-efi-protocol-simple-text-input.h" // for CEfiInputKey, CEfiSim...
#include "efi/c-efi-protocol-simple-text-output.h" // for CEfiSimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for CEfiSystemTable
#include "globals.h"
#include "memory/boot-functions.h"
#include "memory/definitions.h"
#include "memory/standard.h"
#include "printing.h"

// static size_t max_io_apics = 0;
//
// // Following function based on
// // https://github.com/managarm/lai/blob/master/helpers/pc-bios.c's function
// // lai_bios_calc_checksum()
// CEfiU8 acpi_checksum(void *ptr, size_t size) {
//     CEfiU8 sum = 0, *_ptr = ptr;
//     for (CEfiUSize i = 0; i < size; i++) {
//         sum += _ptr[i];
//     }
//     return sum;
// }
//
// void *acpi_get_rsdp(void) {
//     CEfiGuid acpi_2_guid = C_EFI_ACPI_20_TABLE_GUID;
//     CEfiGuid acpi_1_guid = C_ACPI_TABLE_GUID;
//
//     void *rsdp = C_EFI_NULL;
//
//     for (size_t i = 0; i < globals.st->number_of_table_entries; i++) {
//         CEfiConfigurationTable *cur_table =
//         &globals.st->configuration_table[i];
//
//         bool is_xsdp = memcmp(&cur_table->vendor_guid, &acpi_2_guid,
//                               sizeof(CEfiGuid)) == 0;
//         bool is_rsdp = memcmp(&cur_table->vendor_guid, &acpi_1_guid,
//                               sizeof(CEfiGuid)) == 0;
//
//         if (!is_xsdp && !is_rsdp) {
//             continue;
//         }
//
//         if ((is_xsdp &&
//              acpi_checksum(cur_table->vendor_table, sizeof(CAcpiRSDP)) !=
//                  0) || // XSDP is 36 bytes wide
//             (is_rsdp && acpi_checksum(cur_table->vendor_table, 20) !=
//                             0)) // RSDP is 20 bytes wide
//         {
//             continue;
//         }
//
//         globals.st->con_out->output_string(globals.st->con_out,
//                                            u"ACPI: Found ");
//         if (is_xsdp) {
//             globals.st->con_out->output_string(globals.st->con_out, u"XSDP");
//         } else {
//             globals.st->con_out->output_string(globals.st->con_out, u"RSDP");
//         }
//         globals.st->con_out->output_string(globals.st->con_out, u" at ");
//         printNumber((CEfiU64)cur_table->vendor_table, 16);
//         globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
//
//         // We want to return the XSDP if it exists rather then returning
//         // the RSDP. We need to add a check for that since the table entries
//         // are not in the same order for all EFI systems since it might be
//         the
//         // case where the RSDP ocurs before the XSDP.
//         if (is_xsdp) {
//             rsdp = (void *)cur_table->vendor_table;
//             break; // Found it!.
//         } else {
//             // Found the RSDP but we continue to loop since we might
//             // find the XSDP.
//             rsdp = (void *)cur_table->vendor_table;
//         }
//     }
//
//     return rsdp;
// }
//
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
//
// void init_io_apics(void) {
//     static bool already_inited = false;
//     if (already_inited) {
//         return;
//     }
//
//     CAcpiMADT *madt = acpi_get_table("APIC", 0);
//
//     if (madt == C_EFI_NULL) {
//         goto out;
//     }
//
//     for (CEfiU8 *madt_ptr = (CEfiU8 *)madt->madt_entries_begin;
//          (CEfiUSize)madt_ptr < (CEfiUSize)madt + madt->header.length;
//          madt_ptr += *(madt_ptr + 1)) {
//         switch (*madt_ptr) {
//         case 1: {
//             max_io_apics++;
//             continue;
//         }
//         }
//     }
//
//     io_apics = ext_mem_alloc(max_io_apics * sizeof(struct madt_io_apic *));
//     max_io_apics = 0;
//
//     for (CEfiU8 *madt_ptr = (CEfiU8 *)madt->madt_entries_begin;
//          (CEfiUSize)madt_ptr < (CEfiUSize)madt + madt->header.length;
//          madt_ptr += *(madt_ptr + 1)) {
//         switch (*madt_ptr) {
//         case 1: {
//             io_apics[max_io_apics++] = (void *)madt_ptr;
//             continue;
//         }
//         }
//     }
//
// out:
//     already_inited = true;
// }
//
static inline void wrmsr(CEfiU32 msr, CEfiU64 value) {
    CEfiU32 edx = value >> 32;
    CEfiU32 eax = (CEfiU32)value;
    asm volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(msr) : "memory");
}

static inline void outb(CEfiU16 port, CEfiU8 value) {
    asm volatile("outb %%al, %1" : : "a"(value), "Nd"(port) : "memory");
}

void pic_mask_all(void) {
    outb(0xa1, 0xff);
    outb(0x21, 0xff);
}
//
// void io_apic_mask_all(void) {
//     for (size_t i = 0; i < max_io_apics; i++) {
//         CEfiU32 gsi_count = io_apic_gsi_count(i);
//         for (CEfiU32 j = 0; j < gsi_count; j++) {
//             CEfiU64 ioredtbl = j * 2 + 16;
//             switch ((io_apic_read(i, ioredtbl) >> 8) & 0b111) {
//             case 0b000: // Fixed
//             case 0b001: // Lowest Priority
//                 break;
//             default:
//                 continue;
//             }
//
//             io_apic_write(i, ioredtbl, (1 << 16)); // mask
//             io_apic_write(i, ioredtbl + 1, 0);
//         }
//     }
// }

static CEfiU8 in_exc = 0;

// Not sure what we are doing when we encounter an exception tbh.
void fw_exc(CEfiU8 excno, CEfiU64 exccode, CEfiU64 rip, CEfiU64 rsp) {
    CEfiU64 cr2, cr3;
    if (!in_exc) {
        in_exc++;
        __asm__ __volatile__("movq %%cr2, %%rax;movq %%cr3, %%rbx;"
                             : "=a"(cr2), "=b"(cr3)::);
        error(u"Ran into the first exception?\r\n");
    }
    error(u"Ran into the second exception????\r\n");
    __asm__ __volatile__("1: cli; hlt; jmp 1b");
}

#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

void flo_printToScreen(CEfiPhysicalAddress graphics, CEfiU32 color) {
    for (CEfiU64 x = 0; x < 100; x++) {
        ((CEfiU32 *)graphics)[x] = color;
    }
}

typedef struct {
    CEfiU64 ptr;
    CEfiU64 size;
    CEfiU32 columns;
    CEfiU32 rows;
    CEfiU32 scanline;
} FrameBuffer;

typedef struct {
    CEfiU64 ptr;
    CEfiU64 size;
} MemoryMap;

typedef struct {
    FrameBuffer fb;
    MemoryMap *memory;
} KernelParameters;

void jumpIntoKernel() {
    CEfiGuid gop_guid = C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    CEfiGraphicsOutputProtocol *gop = C_EFI_NULL;

    CEfiStatus status = globals.st->boot_services->locate_protocol(
        &gop_guid, C_EFI_NULL, (void **)&gop);
    if (C_EFI_ERROR(status)) {
        error(u"Could not locate locate GOP\r\n");
    }

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"The graphics buffer location is at ");
    printNumber(gop->mode->frameBufferBase, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    CEfiPhysicalAddress kernelParams = allocAndZero(1);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, PAGE_SIZE);
    KernelParameters *params = (KernelParameters *)kernelParams;

    CEfiPhysicalAddress stackEnd = allocAndZero(4);
    mapMemory(stackEnd, STACK_SIZE);
    CEfiPhysicalAddress stackPointer = stackEnd + STACK_SIZE - RED_ZONE_SIZE;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"The stack will go down from ");
    printNumber(stackPointer, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"to ");
    printNumber(stackEnd, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    params->fb.columns = gop->mode->info->horizontalResolution;
    params->fb.rows = gop->mode->info->verticalResolution;
    params->fb.scanline = gop->mode->info->pixelsPerScanLine;
    params->fb.ptr = gop->mode->frameBufferBase;
    params->fb.size = gop->mode->frameBufferSize;

    globals.st->con_out->output_string(
        globals.st->con_out,
        u"Starting exit boot services process, no printing after this!\r\n");

    MemoryInfo memoryInfo = getMemoryInfo();
    status = globals.st->boot_services->exit_boot_services(globals.h,
                                                           memoryInfo.mapKey);

    if (C_EFI_ERROR(status)) {
        globals.st->con_out->output_string(
            globals.st->con_out, u"First exit boot services failed..\r\n");
        status = globals.st->boot_services->free_pages(
            (CEfiPhysicalAddress)memoryInfo.memoryMap,
            EFI_SIZE_TO_PAGES(memoryInfo.memoryMapSize));
        if (C_EFI_ERROR(status)) {
            error(u"Could not free allocated memory map\r\n");
        }

        memoryInfo = getMemoryInfo();
        status = globals.st->boot_services->exit_boot_services(
            globals.h, memoryInfo.mapKey);
    }
    if (C_EFI_ERROR(status)) {
        error(u"could not exit boot services!\r\n");
    }

    /* now that we have left the firmware realm behind, we can get some real
     * work done :-) */
    __asm__ __volatile__(
        /* fw_loadseg might have altered the paging tables for higher-half
           kernels. Better to reload */
        /* CR3 to kick the MMU, but on UEFI we can only do this after we have
           called ExitBootServices */
        "movq %%rax, %%cr3;"
        /* Set up dummy exception handlers */
        //        ".byte 0xe8;.long 0;" /* absolute address to set the code
        //        segment
        //                                 register) */
        //        "1:popq %%rax;"
        //        "movq %%rax, %%rsi;addq $4f - 1b, %%rsi;" /* pointer to the
        //        code stubs
        //                                                   */
        //        "movq %%rax, %%rdi;addq $5f - 1b, %%rdi;" /* pointer to IDT */
        //        "addq $3f - 1b, %%rax;addq %%rax, 2(%%rax);lgdt (%%rax);" /*
        //        we must set
        //                                                                     up a new
        //                                                                     GDT with a
        //                                                                     TSS */
        //        "addq $72, %%rax;" /* patch GDT and load TR */
        //        "movq %%rax, %%rcx;andl $0xffffff, %%ecx;addl %%ecx,
        //        -14(%%rax);" "movq %%rax, %%rcx;shrq $24, %%rcx;movq %%rcx,
        //        -9(%%rax);" "movq $48, %%rax;ltr %%ax;" "movw $32, %%cx;\n" /*
        //        we set up 32 entires in IDT */ "1:movq %%rsi, %%rax;movw
        //        $0x8F01, %%ax;shlq $16, %%rax;movw $32, "
        //        "%%ax;shlq $16, %%rax;movw %%si, %%ax;stosq;"
        //        "movq %%rsi, %%rax;shrq $32, %%rax;stosq;"
        //        "addq $16, %%rsi;decw %%cx;jnz 1b;" /* next entry */
        //        "lidt (6f);jmp 2f;"                 /* set up IDT */
        //        /* new GDT */
        //        ".balign 8;3:;"
        //        ".word 0x40;.long 8;.word 0;.quad 0;" /* value / null
        //        descriptor */
        //        ".long 0x0000FFFF;.long 0x00009800;"  /*   8 - legacy real cs
        //        */
        //        ".long 0x0000FFFF;.long 0x00CF9A00;"  /*  16 - prot mode cs */
        //        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  24 - prot mode ds */
        //        ".long 0x0000FFFF;.long 0x00AF9A00;"  /*  32 - long mode cs */
        //        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  40 - long mode ds */
        //        ".long 0x00000068;.long 0x00008900;"  /*  48 - long mode tss
        //        descriptor
        //                                               */
        //        ".long 0x00000000;.long 0x00000000;"  /*       cont. */
        //        /* TSS */
        //        ".long 0;.long 0x1000;.long 0;.long 0x1000;.long 0;.long
        //        0x1000;.long " "0;"
        //        ".long 0;.long 0;.long 0x1000;.long 0;"
        //        /* ISRs */
        //        "1:popq %%r8;movq 16(%%rsp),%%r9;jmp fw_exc;"
        //        ".balign 16;4:xorq %%rdx, %%rdx; xorb %%cl, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $1, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $2, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $3, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $4, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $5, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $6, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $7, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $8, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $9, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $10, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $11, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $12, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $13, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $14, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $15, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $16, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $17, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $18, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $19, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $20, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $21, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $22, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $23, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $24, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $25, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $26, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $27, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $28, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $29, %%cl;jmp 1b;"
        //        ".balign 16;popq %%rdx; movb $30, %%cl;jmp 1b;"
        //        ".balign 16;xorq %%rdx, %%rdx; movb $31, %%cl;jmp 1b;"
        //        /* IDT */
        //        ".balign 16;5:.space (32*16);"
        //        /* IDT value */
        //        "6:.word 6b-5b;.quad 5b;2:"
        ::"a"(globals.level4PageTable)
        : "rcx", "rsi", "rdi");

    //    void CEFICALL (*entry_point)(KernelParameters *) = (void
    //    *)KERNEL_START; entry_point(params);

    // Set PAT as:
    // PAT0 -> WB  (06)
    // PAT1 -> WT  (04)
    // PAT2 -> UC- (07)
    // PAT3 -> UC  (00)
    // PAT4 -> WP  (05)
    // PAT5 -> WC  (01)
    CEfiU64 pat = (CEfiU64)0x010500070406;
    wrmsr(0x277, pat);

    //    pic_mask_all();

    /* execute 64-bit kernels in long mode */
    __asm__ __volatile__(
        "movq %%rcx, %%r8;"
        /* SysV ABI uses %rdi, %rsi, but fastcall uses %rcx, %rdx */
        //"movq %%rax, %%rcx;movq %%rax, %%rdi;"
        "movq %%rbx, %%rsp; movq %%rsp, %%rbp;;"
        "movq $0x0000FF00, %%rax;" // Load the absolute value
        "movq %%rax, (%%rdx);"     // Store the value at the address
                                   // pointed to by
        "jmp *%%r8" // Jump to the address stored in %%rdx (KERNEL_START)
        ::
            //"a"(params),
        "b"(stackPointer),
        "c"(KERNEL_START), "d"(*(CEfiU32 *)KERNEL_PARAMS_START)
        :);

    __builtin_unreachable();
}

CEFICALL CEfiStatus efi_main(CEfiHandle handle, CEfiSystemTable *systemtable) {
    /* make sure SSE is enabled, because some say there are buggy firmware in
     * the wild not enabling (and also needed if we come from boot_x86.asm). No
     * supported check, because according to AMD64 Spec Vol 2, all long mode
     * capable CPUs must also support SSE2 at least. We don't need them, but
     * it's more than likely that a kernel is compiled using SSE instructions.
     */
    __asm__ __volatile__(
        "movq %%cr0, %%rax;andb $0xF1, %%al;movq %%rax, %%cr0;" /* clear MP, EM,
                                                                   TS (FPU
                                                                   emulation
                                                                   off) */
        "movq %%cr4, %%rax;orw $3 << 9, %%ax;movq %%rax, %%cr4;" /* set OSFXSR,
                                                                    OSXMMEXCPT
                                                                    (enable SSE)
                                                                  */
        ::
            : "rax");

    globals.h = handle;
    globals.st = systemtable;

    globals.st->con_out->reset(globals.st->con_out, false);
    globals.st->con_out->set_attribute(globals.st->con_out,
                                       C_EFI_BACKGROUND_RED | C_EFI_YELLOW);

    globals.level4PageTable = (CEfiPhysicalAddress *)allocAndZero(1);

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to read kernel info\r\n");
    DataPartitionFile kernelFile = getKernelInfo();

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Going to load kernel\r\n");
    AsciString kernelContent = readDiskLbas(
        kernelFile.lbaStart, kernelFile.bytes, getDiskImageMediaID());

    globals.st->con_out->output_string(
        globals.st->con_out, u"Read kernel content, at memory location:");
    printNumber((CEfiUSize)kernelContent.buf, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"\r\n");

    MemoryInfo memoryInfo = getMemoryInfo();
    CEfiMemoryDescriptor *iterator;
    for (CEfiU64 i = 0; i < memoryInfo.memoryMapSize;
         iterator = (CEfiMemoryDescriptor *)((char *)memoryInfo.memoryMap + i),
                 i += memoryInfo.descriptorSize) {
        if (needsTobeMappedByOS(iterator->type)) {
            globals.st->con_out->output_string(globals.st->con_out,
                                               u"Memory entry:");
            printNumber(iterator->physical_start, 16);
            globals.st->con_out->output_string(globals.st->con_out, u" ");
            printNumber(iterator->virtual_start, 16);
            globals.st->con_out->output_string(globals.st->con_out, u" ");
            printNumber(iterator->number_of_pages, 16);
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        }
    }

    //    init_flush_irqs();
    //    init_io_apics();

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Attempting to map memory now...\r\n");

    mapMemoryAt(0, 0, 4294967295); // 4 GiB ???
    mapMemoryAt((CEfiU64)kernelContent.buf, KERNEL_START,
                (CEfiU32)kernelContent.len);

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"Preparing to jump to kernel...\r\n");
    jumpIntoKernel();

    return !C_EFI_SUCCESS;
}
