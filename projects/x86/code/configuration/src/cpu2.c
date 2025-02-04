#include "x86/configuration/cpu2.h"

CPUIDResult CPUID(U32 functionID) {
    CPUIDResult result;
    __asm__ __volatile__("cpuid"
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
    __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    U64 endInCycles = currentCycles + microSeconds * cyclesPerMicroSecond;
    do {
        __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
        currentCycles = ((U64)edx << 32) | eax;
    } while (currentCycles < endInCycles);
}

void disablePICAndNMI() {
    __asm__ __volatile__(
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
