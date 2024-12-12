#include "x86/memory/status.h"
#include "platform-abstraction/cpu.h"
#include "platform-abstraction/log.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86/memory/pat.h"
#include "x86/memory/physical.h"
#include "x86/memory/virtual.h"

static string pageSizeToString(PageSize pageSize) {
    switch (pageSize) {
    case BASE_PAGE: {
        return STRING("Base page frame, 4KiB");
    }
    case LARGE_PAGE: {
        return STRING("Large page, 2MiB");
    }
    default: {
        return STRING("Huge page, 1GiB");
    }
    }
}

static void appendPMMStatus(PhysicalMemoryManager manager) {
    KLOG(STRING("Type: "));
    KLOG(pageSizeToString(manager.pageSize), NEWLINE);
    KLOG(STRING("Used base page frames for internal structure: "));
    KLOG(manager.usedBasePages, NEWLINE);
    KLOG(STRING("Free pages:\t"));
    U64 totalPages = 0;
    for (U64 i = 0; i < manager.memory.len; i++) {
        KLOG(manager.memory.buf[i].numberOfPages);
        KLOG(STRING(" "));
        totalPages += manager.memory.buf[i].numberOfPages;
    }
    KLOG(STRING(" Total: "));
    KLOG(totalPages, NEWLINE);
    KLOG(STRING("Total memory regions:\t"));
    KLOG(manager.memory.len, NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    KLOG(STRING("Physical Memory status\n"));
    KLOG(STRING("================\n"));
    appendPMMStatus(basePMM);
    KLOG(STRING("================\n"));
    appendPMMStatus(largePMM);
    KLOG(STRING("================\n"));
    appendPMMStatus(hugePMM);
    KLOG(STRING("================\n"));
}

static string patEncodingToString[PAT_NUMS] = {
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
