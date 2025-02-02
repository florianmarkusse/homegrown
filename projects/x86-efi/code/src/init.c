#include "efi/error.h"
#include "efi/firmware/base.h"   // for PhysicalAddress
#include "efi/firmware/system.h" // for PhysicalAddress
#include "efi/globals.h"
#include "platform-abstraction/efi.h"
#include "platform-abstraction/log.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "x86-efi/features.h"
#include "x86-efi/gdt.h"

typedef struct {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
} CPUIDResult;
CPUIDResult CPUID(U32 functionID) {
    CPUIDResult result;
    __asm__ __volatile__("cpuid"
                         : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx),
                           "=d"(result.edx)
                         : "a"(functionID)
                         : "cc");
    return result;
}

static U64 microSecondInCycles = 1;
// 1 millionth of a second
void wait(U64 microSeconds) {
    U32 edx;
    U32 eax;
    __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    U64 endInCycles = currentCycles + microSeconds * microSecondInCycles;
    do {
        __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
        currentCycles = ((U64)edx << 32) | eax;
    } while (currentCycles < endInCycles);
}

// NOTE: this should be done per core probably?
static constexpr auto CALIBRATION_MICROSECONDS = 100;
void calibrateWait() {
    U32 edx;
    U32 eax;
    __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    globals.st->boot_services->stall(CALIBRATION_MICROSECONDS);
    __asm__ __volatile__("rdtscp" : "=a"(eax), "=d"(edx));
    U64 endCycles = ((U64)edx << 32) | eax;
    microSecondInCycles = endCycles - currentCycles / CALIBRATION_MICROSECONDS;
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

void bootstrapProcessorWork() {
    disablePICAndNMI();

    prepNewGDT();

    // NOTE: WHY????
    globals.st->boot_services->stall(100000);

    __asm__ __volatile__("pause" : : : "memory"); // memory barrier
}

void messageAndExit(string message) {
    KFLUSH_AFTER {
        ERROR(message, NEWLINE);
        ERROR(STRING("Buy newer CPU.\n"));
    }
    waitKeyThenReset();
}

void initArchitecture() {
    asm volatile("cli");

    KFLUSH_AFTER {
        INFO(STRING("CR3 memory location:"));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)globals.rootPageTable, NEWLINE);
    }

    U32 maxCPUID = CPUID(0).eax;
    if (maxCPUID < 1) {
        messageAndExit(STRING("CPU does not support CPUID of 1 and above"));
    }

    CPUIDResult CPUInfo = CPUID(1);
    features.ecx = CPUInfo.ecx;
    features.edx = CPUInfo.edx;

    if (!features.APIC) {
        messageAndExit(STRING("CPU does not support APIC"));
    }

    U32 BSPID = CPUInfo.ebx >> 24;

    if (!features.TSC) {
        messageAndExit(STRING("CPU does not support Time Stamp Counter"));
    }
    calibrateWait();

    if (!features.PGE) {
        messageAndExit(STRING("CPU does not support global memory paging!"));
    }
    KFLUSH_AFTER { INFO(STRING("Enabling GPE\n")); }
    CPUEnableGPE();

    if (!features.FPU) {
        messageAndExit(STRING("CPU does not support FPU!"));
    }
    KFLUSH_AFTER { INFO(STRING("Enabling FPU\n")); }
    CPUEnableFPU();

    if (!features.SSE) {
        messageAndExit(STRING("CPU does not support SSE!"));
    }
    KFLUSH_AFTER {
        INFO(STRING(
            "Enabling SSE... even though it doesnt work yet anyway lol\n"));
    }
    CPUEnableSSE();

    //   if (!features.XSAVE) {
    //       messageAndExit(STRING("CPU does not support XSAVE!"));
    //   }
    //   KFLUSH_AFTER { INFO(STRING("Enabling XSAVE\n")); }
    //   CPUEnableXSAVE();

    //   if (!features.AVX) {
    //       messageAndExit(STRING("CPU does not support AVX!"));
    //   }
    //   KFLUSH_AFTER { INFO(STRING("Enabling AVX\n")); }
    //   CPUEnableAVX();

    KFLUSH_AFTER { INFO(STRING("Bootstrap processor work\n")); }
    bootstrapProcessorWork();
}
