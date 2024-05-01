#include "gdt.h"

void setupNewGDT() {
    gdt_table.tssDescriptor = (Tss_Descriptor){
        .limit_15_0 = sizeof tss - 1,
        .base_15_0 = tss_address & 0xFFFF,
        .base_23_16 = (tss_address >> 16) & 0xFF,
        .type = 9, // 0b1001 = 64 bit TSS (available)
        .p_flag = 1,
        .base_31_24 = (tss_address >> 24) & 0xFF,
        .base_63_32 = (tss_address >> 32) & 0xFFFFFFFF,

    };

    __asm__ __volatile__(
        "lgdt %0;" // Load new Global Descriptor Table

        "movw $0x18, %%ax;" // 0x18 = tss segment offset in gdt
        "ltr %%ax;"         // Load task register, with tss offset

        "movq $0x08, %%rax;"
        "pushq %%rax;"           // Push kernel code segment
        "leaq 1f(%%rip), %%rax;" // Use relative offset for label
        "pushq %%rax;"           // Push return address
        "lretq;"                 // Far return, pop return address into IP,
                                 //   and pop code segment into CS
        "1:"
        "movw $0x10, %%ax;" // 0x10 = kernel data segment
        "movw %%ax, %%ds;"  // Load data segment registers
        "movw %%ax, %%es;"
        "movw %%ax, %%fs;"
        "movw %%ax, %%gs;"
        "movw %%ax, %%ss;"

        :
        : "m"(gdtr)
        : "rax", "memory");
}
