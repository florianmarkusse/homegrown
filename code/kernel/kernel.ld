/* Kernel Linker Script */
ENTRY(main)
SECTIONS {
    /* 32Kbi */
    . = 0x8000;
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
