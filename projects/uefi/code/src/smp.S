.globl ap_trampoline
.extern bootboot_startcode

/*****************************************************************************
 * things to do on the APs                                                   *
 *****************************************************************************/
    .balign 128
    .code16
    /* this code will be relocated to 0x8000 - 0x8100 */
ap_trampoline:
    cli
    cld
    ljmp    $0, $0x8040
    .balign 16
    // prot mode GDT
_L8010_GDT_table:
    .long 0, 0
    .long 0x0000FFFF, 0x00CF9A00    // flat code
    .long 0x0000FFFF, 0x008F9200    // flat data
    .long 0x00000068, 0x00CF8900    // tss, not used but required by VB's vt-x
_L8030_GDT_value:
    .word _L8030_GDT_value - _L8010_GDT_table - 1
    .long 0x8010
    .long 0, 0
    .balign 64
_L8040:
    xorw    %ax, %ax
    movw    %ax, %ds
    lgdtl   0x8030
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    ljmp    $8, $0x8060
    .balign 32
    .code32
_L8060:
    movw    $16, %ax
    movw    %ax, %ds
    movw    %ax, %ss
    movl    $0x368, %eax        // Set PAE, MCE, PGE; OSFXSR, OSXMMEXCPT (enable SSE)
    movl    %eax, %cr4
    movl    0x80C0, %eax        // let's hope it's in the first 4G...
    movl    %eax, %cr3
    movl    $0x0C0000080, %ecx  // EFR MSR
    rdmsr
    orl     $0x100, %eax        // enable long mode
    wrmsr
    movl    $0x0C0000011, %eax  // clear EM, MP (enable SSE) and WP
    movl    %eax, %cr0
    lgdtl   0x80E0
    movl    $0x80C8, %esp       // we can't use "ljmp $8, $0x80A0", because we don't know cs
    lret
    .balign 32
    .code64
_L80A0:
    movl    0x80D0, %eax         // load long mode segments
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs
// some linkers (GNU ld) generates bad relocation record for this
//    jmp     bootboot_startcode
    movq    0x80D8, %rax
    // in theory this could cause trouble, but it does not since all cores are executing the same
    // code at this point, so it doesn't matter if one core is overwriting the same stack with the
    // stack frame, because all are saving exactly the same stack frame to the same position
    movl    $0x8800, %esp
    jmp     *%rax
    .balign 32
_L80C0_cr3_value:
    .long 0, 0
    .long 0x80A0
_L80CC_cs_value:
    .long 0
_L80D0_ds_value:
    .long 0, 0
_L80D8_bootboot_startcore:
    .long 0, 0
_L80E0_gdt_value:
    .long 0, 0, 0, 0
ap_trampoline_end:

