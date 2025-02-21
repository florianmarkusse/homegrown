#include "abstraction/log.h"
#include "abstraction/memory/physical/allocation.h"
#include "efi/error.h"
#include "efi/firmware/base.h"   // for PhysicalAddress
#include "efi/firmware/system.h" // for PhysicalAddress
#include "efi/globals.h"
#include "efi/memory.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/text/string.h"
#include "x86/efi/gdt.h"
#include "x86/memory/virtual.h"
#include "x86/configuration/cpu2.h"
#include "x86/configuration/features.h"
#include "x86/gdt.h"

void bootstrapProcessorWork() {
    disablePICAndNMI();

    Status status = globals.st->boot_services->allocate_pages(
        ALLOCATE_ANY_PAGES, LOADER_DATA,
        CEILING_DIV_VALUE(3 * sizeof(PhysicalBasePage), UEFI_PAGE_SIZE),
        &gdtData);
    EXIT_WITH_MESSAGE_IF(status) {
        ERROR(STRING("Could not allocate data for disk buffer\n"));
    }

    gdtDescriptor = prepNewGDT((PhysicalBasePage *)gdtData);

    // NOTE: WHY????
    globals.st->boot_services->stall(100000);

    asm volatile("pause" : : : "memory"); // memory barrier
}

// NOTE: this should be done per core probably?
static constexpr auto CALIBRATION_MICROSECONDS = 100;
void calibrateWait() {
    U32 edx;
    U32 eax;
    asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
    U64 currentCycles = ((U64)edx << 32) | eax;
    globals.st->boot_services->stall(CALIBRATION_MICROSECONDS);
    asm volatile("rdtscp" : "=a"(eax), "=d"(edx));
    U64 endCycles = ((U64)edx << 32) | eax;
    cyclesPerMicroSecond = endCycles - currentCycles / CALIBRATION_MICROSECONDS;
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

    {
        U64 newCR3 = allocate4KiBPage(1);
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        memset((void *)newCR3, 0, UEFI_PAGE_SIZE);
        level4PageTable = (VirtualPageTable *)newCR3;
    }

    KFLUSH_AFTER {
        INFO(STRING("root page table memory location:"));
        /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
        INFO((void *)level4PageTable, NEWLINE);
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

    // TODO: THE PAT PROGRAMMING HERE!

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
