#ifndef X86_CONFIGURATION_CPU2_H
#define X86_CONFIGURATION_CPU2_H

// FIX: Once the platform-abstraction stuff is moved, merge this with cpu.c

#include "shared/types/types.h"
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
#endif
