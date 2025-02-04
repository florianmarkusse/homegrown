#include "x86/configuration/features.h"
#include "shared/types/types.h"

CPUFeatures features;

typedef struct {
    union {
        U64 value;
        struct {
            U64 PE : 1;
            U64 MP : 1;
            U64 EM : 1;
            U64 TS : 1;
            U64 ET : 1;
            U64 NE : 1;
            U64 reserved : 10;
            U64 WP : 1;
            U64 reserved1 : 1;
            U64 AM : 1;
            U64 reserved2 : 10;
            U64 NW : 1;
            U64 CD : 1;
            U64 PG : 1;
            U64 reserved3 : 32;
        };
    };
} CR0;

typedef struct {
    union {
        U64 value;
        struct {
            U64 VME : 1;
            U64 PVI : 1;
            U64 TSD : 1;
            U64 DE : 1;
            U64 PSE : 1;
            U64 PAE : 1;
            U64 MCE : 1;
            U64 PGE : 1;
            U64 PCE : 1;
            U64 OSFXSR : 1;
            U64 OSXMMEXCPT : 1;
            U64 UMIP : 1;
            U64 LA57 : 1;
            U64 VMXE : 1;
            U64 SMXE : 1;
            U64 reserved : 1;
            U64 FSGSBASE : 1;
            U64 PCIDE : 1;
            U64 OSXSAVE : 1;
            U64 KL : 1;
            U64 SMEP : 1;
            U64 SMAP : 1;
            U64 PKE : 1;
            U64 CET : 1;
            U64 PKS : 1;
            U64 UINTR : 1;
            U64 reserved1 : 38;
        };
    };
} CR4;

typedef struct {
    union {
        U64 value;
        struct {
            U32 edx;
            U32 eax;
        };
        struct {
            U64 FPU_MMX : 1;
            U64 SSE : 1;
            U64 AVX : 1;
            U64 BNDREG : 1;
            U64 BNDCSR : 1;
            U64 opmask : 1;
            U64 ZMM_Hi256 : 1;
            U64 Hi16_ZMM : 1;
            U64 reserved : 1;
            U64 PKRU : 1;
            U64 reserved1 : 7;
            U64 TILECONFIG : 1;
            U64 TILEDATA : 1;
            U64 reserved2 : 44;
            U64 expansion : 1;
        };
    };
} XCR0;

void CPUEnableGPE() {
    CR4 cr4;

    // Read CR4 register
    __asm__ __volatile__("mov %%cr4, %%rax" : "=a"(cr4));

    // Set Operating System Support for Page Global Enable
    cr4.PGE = 1;

    // Write the modified CR4 register back
    __asm__ __volatile__("mov %%rax, %%cr4" : : "a"(cr4));
}

void CPUEnableFPU() {
    CR0 cr0;

    // Read CR0 register
    __asm__ __volatile__("mov %%cr0, %%rax" : "=a"(cr0));

    // Set Monitor co-processor
    cr0.MP = 1;

    // Clear Emulation
    cr0.EM = 0;

    // Set Native Exception
    cr0.NE = 1;

    // Clear TS
    cr0.TS = 0;

    // Write the modified CR0 register back
    asm volatile("mov %%rax, %%cr0" : : "a"(cr0));

    // Initialize the FPU (reset the FPU control word)
    asm volatile("finit" : : : "st");
}

void CPUEnableXSAVE() {
    CR4 cr4;
    XCR0 xcr0;

    // Read CR4 register
    asm volatile("mov %%cr4, %%rax" : "=a"(cr4));

    // Set Operating System Support for XSave
    cr4.OSXSAVE = 1;

    // Write the modified CR4 register back
    asm volatile("mov %%rax, %%cr4" : : "a"(cr4));

    // Set the control register
    asm volatile("xgetbv" : "=a"(xcr0.eax), "=d"(xcr0.edx) : "c"(0));

    // NOTE: This will need to be changed once we support AVX-512
    // Turn on AVX and SSE state
    xcr0.AVX = 1;
    xcr0.SSE = 1;

    // Write the modified XCR0 register
    asm volatile("xsetbv" : : "c"(0), "a"(xcr0.eax), "d"(xcr0.edx));
}

void CPUEnableAVX() {
    CR4 cr4;

    // Read CR4 register
    asm volatile("mov %%cr4, %%rax" : "=a"(cr4) :);

    // Set Operating System Support for Advanced Vector Extensions
    cr4.SMXE = 1;

    // Write the modified CR4 register back
    asm volatile("mov %%rax, %%cr4" : : "a"(cr4));
}

void CPUEnableSSE() {
    CR4 cr4;

    // Read CR4 register
    asm volatile("mov %%cr4, %%rax" : "=a"(cr4));

    // Set Operating System Support for FXSAVE and FXSTOR instructions
    cr4.OSFXSR = 1;

    // Set Operating System Support for Unmasked SIMD Floating-Point Exceptions
    // (Bit 10)
    cr4.OSXMMEXCPT = 1;

    // Write the modified CR4 register back
    asm volatile("mov %%rax, %%cr4" : : "a"(cr4));
}
