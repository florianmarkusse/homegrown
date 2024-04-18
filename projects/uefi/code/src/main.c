#include "data-reading.h"
#include "efi/c-efi-base.h" // for CEfiStatus, C_EFI_SUC...
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

typedef struct {
    CEfiU32 columns;
    CEfiU32 rows;
    CEfiU32 scanline;
    CEfiU64 ptr;
    CEfiU64 size;
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

    CEfiPhysicalAddress kernelParams = allocAndZero(1);
    mapMemoryAt(kernelParams, KERNEL_PARAMS_START, PAGE_SIZE);
    KernelParameters *params = (KernelParameters *)kernelParams;

    CEfiPhysicalAddress stackEnd = allocAndZero(4);
    mapMemory(stackEnd, 4 * PAGE_SIZE);
    CEfiPhysicalAddress stackPointer = stackEnd + 4 * PAGE_SIZE;

    globals.st->con_out->output_string(globals.st->con_out,
                                       u"The stack will go down from ");
    printNumber(stackPointer, 16);
    globals.st->con_out->output_string(globals.st->con_out, u"to ");
    printNumber(stackPointer, 16);
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
        ".byte 0xe8;.long 0;" /* absolute address to set the code segment
                                 register) */
        "1:popq %%rax;"
        "movq %%rax, %%rsi;addq $4f - 1b, %%rsi;" /* pointer to the code stubs
                                                   */
        "movq %%rax, %%rdi;addq $5f - 1b, %%rdi;" /* pointer to IDT */
        "addq $3f - 1b, %%rax;addq %%rax, 2(%%rax);lgdt (%%rax);" /* we must set
                                                                     up a new
                                                                     GDT with a
                                                                     TSS */
        "addq $72, %%rax;" /* patch GDT and load TR */
        "movq %%rax, %%rcx;andl $0xffffff, %%ecx;addl %%ecx, -14(%%rax);"
        "movq %%rax, %%rcx;shrq $24, %%rcx;movq %%rcx, -9(%%rax);"
        "movq $48, %%rax;ltr %%ax;"
        "movw $32, %%cx;\n" /* we set up 32 entires in IDT */
        "1:movq %%rsi, %%rax;movw $0x8F01, %%ax;shlq $16, %%rax;movw $32, "
        "%%ax;shlq $16, %%rax;movw %%si, %%ax;stosq;"
        "movq %%rsi, %%rax;shrq $32, %%rax;stosq;"
        "addq $16, %%rsi;decw %%cx;jnz 1b;" /* next entry */
        "lidt (6f);jmp 2f;"                 /* set up IDT */
        /* new GDT */
        ".balign 8;3:;"
        ".word 0x40;.long 8;.word 0;.quad 0;" /* value / null descriptor */
        ".long 0x0000FFFF;.long 0x00009800;"  /*   8 - legacy real cs */
        ".long 0x0000FFFF;.long 0x00CF9A00;"  /*  16 - prot mode cs */
        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  24 - prot mode ds */
        ".long 0x0000FFFF;.long 0x00AF9A00;"  /*  32 - long mode cs */
        ".long 0x0000FFFF;.long 0x00CF9200;"  /*  40 - long mode ds */
        ".long 0x00000068;.long 0x00008900;"  /*  48 - long mode tss descriptor
                                               */
        ".long 0x00000000;.long 0x00000000;"  /*       cont. */
        /* TSS */
        ".long 0;.long 0x1000;.long 0;.long 0x1000;.long 0;.long 0x1000;.long "
        "0;"
        ".long 0;.long 0;.long 0x1000;.long 0;"
        /* ISRs */
        "1:popq %%r8;movq 16(%%rsp),%%r9;jmp fw_exc;"
        ".balign 16;4:xorq %%rdx, %%rdx; xorb %%cl, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $1, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $2, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $3, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $4, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $5, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $6, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $7, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $8, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $9, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $10, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $11, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $12, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $13, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $14, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $15, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $16, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $17, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $18, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $19, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $20, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $21, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $22, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $23, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $24, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $25, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $26, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $27, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $28, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $29, %%cl;jmp 1b;"
        ".balign 16;popq %%rdx; movb $30, %%cl;jmp 1b;"
        ".balign 16;xorq %%rdx, %%rdx; movb $31, %%cl;jmp 1b;"
        /* IDT */
        ".balign 16;5:.space (32*16);"
        /* IDT value */
        "6:.word 6b-5b;.quad 5b;2:" ::"a"(globals.level4PageTable)
        : "rcx", "rsi", "rdi");

    //    void CEFICALL (*entry_point)(KernelParameters *) = (void
    //    *)KERNEL_START; entry_point(params);

    /* execute 64-bit kernels in long mode */
    __asm__ __volatile__(
        "movq %%rcx, %%r8;"
        /* SysV ABI uses %rdi, %rsi, but fastcall uses %rcx, %rdx */
        //"movq %%rax, %%rcx;movq %%rax, %%rdi;"
        "movq %%rbx, %%rsp; movq %%rsp, %%rbp;;"
        "jmp *%%r8" // Jump to the address stored in %%rdx (KERNEL_START)
        ::
            //"a"(params),
        "b"(stackPointer),
        "c"(KERNEL_START)
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
