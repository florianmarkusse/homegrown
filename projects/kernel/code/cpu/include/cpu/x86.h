#ifndef CPU_X86_H
#define CPU_X86_H

#include "interoperation/types.h"

U64 rdmsr(U32 msr);
void wrmsr(U32 msr, U64 value);

void flushTLB();

#endif
