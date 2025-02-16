#ifndef X86_CONFIGURATION_CPU2_H
#define X86_CONFIGURATION_CPU2_H

// FIX: Once the platform-abstraction stuff is moved, merge this with cpu.c

#include "shared/types/types.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushTLB();
void flushCPUCaches();

extern U64 cyclesPerMicroSecond;
void wait(U64 microSeconds);

typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
} CPUIDResult;
CPUIDResult CPUID(U32 functionID);
void disablePICAndNMI();
U64 CR3();

#endif
