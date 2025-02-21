#ifndef X86_CONFIGURATION_FEATURES_H
#define X86_CONFIGURATION_FEATURES_H

#include "shared/types/types.h"

typedef struct {
    union {
        U32 ecx;
        struct {
            U32 SSE3 : 1;
            U32 PCLMULQDQ : 1;
            U32 DTES64 : 1;
            U32 MONITOR : 1;
            U32 DS_CPL : 1;
            U32 VMX : 1;
            U32 SMX : 1;
            U32 EIST : 1;
            U32 TM2 : 1;
            U32 SSSE3 : 1;
            U32 CNXT_ID : 1;
            U32 SDBG : 1;
            U32 FMA : 1;
            U32 CMPXCHG16B : 1;
            U32 xTPR_Update_Control : 1;
            U32 PDCM : 1;
            U32 reserved : 1;
            U32 PCID : 1;
            U32 DCA : 1;
            U32 SSE4_1 : 1;
            U32 SSE4_2 : 1;
            U32 x2APIC : 1;
            U32 MOVBE : 1;
            U32 POPCNT : 1;
            U32 TSC_DEADLINE : 1;
            U32 AESNI : 1;
            U32 XSAVE : 1;
            U32 OSXSAVE : 1;
            U32 AVX : 1;
            U32 F16C : 1;
            U32 RDRAND : 1;
            U32 reserved1 : 1;
        };
    };
    union {
        U32 edx;
        struct {
            U32 FPU : 1;
            U32 VME : 1;
            U32 DE : 1;
            U32 PSE : 1;
            U32 TSC : 1;
            U32 MSR : 1;
            U32 PAE : 1;
            U32 MCE : 1;
            U32 CX8 : 1;
            U32 APIC : 1;
            U32 reserved2 : 1;
            U32 SEP : 1;
            U32 MTRR : 1;
            U32 PGE : 1;
            U32 MCA : 1;
            U32 CMOV : 1;
            U32 PAT : 1;
            U32 PSE_36 : 1;
            U32 PSN : 1;
            U32 CLFSH : 1;
            U32 reserved3 : 1;
            U32 DS : 1;
            U32 ACPI : 1;
            U32 MMX : 1;
            U32 FXSR : 1;
            U32 SSE : 1;
            U32 SSE2 : 1;
            U32 SS : 1;
            U32 HTT : 1;
            U32 TM : 1;
            U32 reserved4 : 1;
            U32 PBE : 1;
        };
    };
} CPUFeatures;

extern CPUFeatures features;

void CPUEnableGPE();
void CPUEnableFPU();
void CPUEnableXSAVE();
void CPUEnableAVX();
void CPUEnableSSE();

#endif
