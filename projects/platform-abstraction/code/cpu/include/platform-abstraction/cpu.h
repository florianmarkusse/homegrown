#ifndef PLATFORM_ABSTRACTION_CPU_H
#define PLATFORM_ABSTRACTION_CPU_H

#include "shared/types/types.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushTLB();
void flushCPUCaches();

#endif
