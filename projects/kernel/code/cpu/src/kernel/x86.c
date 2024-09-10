#include "cpu/x86.h"
#include "interoperation/types.h"

U64 rdmsr(U32 msr) {
    U32 edx;
    U32 eax;
    asm volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(msr) : "memory");
    return ((U64)edx << 32) | eax;
}

void wrmsr(U32 msr, U64 value) {
    U32 edx = value >> 32;
    U32 eax = (U32)value;
    asm volatile("wrmsr" : : "a"(eax), "d"(edx), "c"(msr) : "memory");
}

void flushTLB() {
    U64 cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3)::"memory");
    asm volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

void flushCPUCaches() { asm volatile("wbinvd" ::: "memory"); }
