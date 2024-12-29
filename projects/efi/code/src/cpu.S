.section .text
.code64

# Assembly routine to enable xsave support
.globl CpuEnableXSave
CpuEnableXSave:
    mov %cr4, %rax
    bts $18, %rax            # Set Operating System Support for XSave (Bit 18)
    mov %rax, %cr4

    # Set the control register
    mov $0, %rcx
    xgetbv
    orl $6, %eax
    mov $0, %rcx
    xsetbv
    ret

# Assembly routine to enable avx support
.globl CpuEnableAvx
CpuEnableAvx:
    mov %cr4, %rax
    bts $14, %rax            # Set Operating System Support for Advanced Vector Extensions (Bit 14)
    mov %rax, %cr4
    ret

# Assembly routine to enable sse support
.globl CpuEnableSse
CpuEnableSse:
    mov %cr4, %rax
    bts $9, %rax             # Set Operating System Support for FXSAVE and FXSTOR instructions (Bit 9)
    bts $10, %rax            # Set Operating System Support for Unmasked SIMD Floating-Point Exceptions (Bit 10)
    mov %rax, %cr4
    ret

# Assembly routine to enable fpu support
.globl CpuEnableFpu
CpuEnableFpu:
    mov %cr0, %rax
    bts $1, %rax             # Set Monitor co-processor (Bit 1)
    btr $2, %rax             # Clear Emulation (Bit 2)
    bts $5, %rax             # Set Native Exception (Bit 5)
    btr $3, %rax             # Clear TS
    mov %rax, %cr0

    finit
    ret

# Assembly routine to enable global page support
.globl CpuEnableGpe
CpuEnableGpe:
    mov %cr4, %rax
    bts $7, %rax             # Set Operating System Support for Page Global Enable (Bit 7)
    mov %rax, %cr4
    ret
