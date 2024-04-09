/* Kernel Linker Script */
ENTRY(kernelmain)
SECTIONS {
    . = 0x0;
    kernel-start : {
        KEEP(*(.kernel-start))
    }
    kernel.text : {
        *(.text .text.*)
    }
    . = ALIGN(4k);
    kernel.bss : {
        *(.bss .bss.*)
    }
    kernel.data : {
        *(.data .data.*)
    }
    . = ALIGN(4k);
    kernel.rodata : {
        *(.rodata .rodata.*)
    }
    . = ALIGN(4k);
    kern_end = .;
    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
