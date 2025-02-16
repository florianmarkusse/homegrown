#include "x86/configuration/cpu2.h"

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

CPUIDResult CPUID(U32 functionID) {
    CPUIDResult result;
    asm volatile("cpuid"
                 : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx),
                   "=d"(result.edx)
                 : "a"(functionID)
                 : "cc");
    return result;
}

U64 cyclesPerMicroSecond = 1;
// 1 millionth of a second
void wait(U64 microSeconds) {
    U32 edx;
    U32 eax;
    asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    U64 endInCycles = currentCycles + microSeconds * cyclesPerMicroSecond;
    do {
        asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
        currentCycles = ((U64)edx << 32) | eax;
    } while (currentCycles < endInCycles);
}

void disablePICAndNMI() {
    asm volatile(
        "movb $0xFF, %%al;" // Set AL to 0xFF
        "outb %%al, $0x21;" // Disable master PIC
        "outb %%al, $0xA1;" // Disable slave PIC
        "inb $0x70, %%al;"  // Read from port 0x70
        "orb $0x80, %%al;"  // Set the NMI disable bit (bit 7)
        "outb %%al, $0x70;" // Write the modified value back to port 0x70
        :                   // No output operands
        :                   // No input operands
        : "eax", "memory"   // Clobbered registers: eax and memory
    );
}

U64 CR3() {
    U64 cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}
