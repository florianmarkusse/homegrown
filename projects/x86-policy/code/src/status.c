#include "platform-abstraction/memory/management/status.h"

#include "platform-abstraction/cpu.h"
#include "platform-abstraction/log.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86/memory/definitions/virtual.h"
#include "x86/memory/pat.h"
#include "x86/memory/physical.h"
#include "x86/memory/virtual.h"

static string patEncodingToString[PAT_ENCODING_COUNT] = {
    STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
    STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
    STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
    STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
};

static void appendVirtualRegionStatus(VirtualRegion region) {
    KLOG(STRING("Start: "));
    KLOG((void *)region.start, NEWLINE);
    KLOG(STRING("End: "));
    KLOG((void *)region.end, NEWLINE);
}

void appendVirtualMemoryManagerStatus() {
    KLOG(STRING("Available Virtual Memory\n"));
    KLOG(STRING("Lower half (0x0000_000000000000):\n"));
    appendVirtualRegionStatus(lowerHalfRegion);
    KLOG(STRING("Higher half(0xFFFF_000000000000):\n"));
    appendVirtualRegionStatus(higherHalfRegion);

    KLOG(STRING("CR3/root page table address is: "));
    KLOG((void *)level4PageTable, NEWLINE);

    PAT patValues = {.value = rdmsr(PAT_LOCATION)};
    KLOG(STRING("PAT MSR set to:\n"));
    for (U8 i = 0; i < 8; i++) {
        KLOG(STRING("PAT "));
        KLOG(i);
        KLOG(STRING(": "));
        KLOG(patEncodingToString[patValues.pats[i].pat], NEWLINE);
    }
}
