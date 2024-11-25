#include "cpu/x86.h"
#include "interoperation/types.h"
#include "memory/management/physical.h"
#include "memory/management/virtual.h"
#include "platform-abstraction/log.h"
#include "shared/text/string.h"
#include "status/memory/status.h"
#include "x86/memory/virtual.h"
#include "x86/memory/pat.h"

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
    LOG(STRING("Type: "));
    LOG(pageSizeToString(manager.pageSize), NEWLINE);
    LOG(STRING("Used base page frames for internal structure: "));
    LOG(manager.usedBasePages, NEWLINE);
    LOG(STRING("Free pages:\t"));
    U64 totalPages = 0;
    for (U64 i = 0; i < manager.memory.len; i++) {
        LOG(manager.memory.buf[i].numberOfPages);
        LOG(STRING(" "));
        totalPages += manager.memory.buf[i].numberOfPages;
    }
    LOG(STRING(" Total: "));
    LOG(totalPages, NEWLINE);
    LOG(STRING("Total memory regions:\t"));
    LOG(manager.memory.len, NEWLINE);
}

void appendPhysicalMemoryManagerStatus() {
    LOG(STRING("Physical Memory status\n"));
    LOG(STRING("================\n"));
    appendPMMStatus(basePMM);
    LOG(STRING("================\n"));
    appendPMMStatus(largePMM);
    LOG(STRING("================\n"));
    appendPMMStatus(hugePMM);
    LOG(STRING("================\n"));
}

static string patEncodingToString[PAT_NUMS] = {
    STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
    STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
    STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
    STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
};

static void appendVirtualRegionStatus(VirtualRegion region) {
    LOG(STRING("Start: "));
    LOG((void *)region.start, NEWLINE);
    LOG(STRING("End: "));
    LOG((void *)region.end, NEWLINE);
}

void appendVirtualMemoryManagerStatus() {
    LOG(STRING("Available Virtual Memory\n"));
    LOG(STRING("Lower half (0x0000_000000000000):\n"));
    appendVirtualRegionStatus(lowerHalfRegion);
    LOG(STRING("Higher half(0xFFFF_000000000000):\n"));
    appendVirtualRegionStatus(higherHalfRegion);

    LOG(STRING("CR3/root page table address is: "));
    LOG((void *)level4PageTable, NEWLINE);

    PAT patValues = {.value = rdmsr(PAT_LOCATION)};
    LOG(STRING("PAT MSR set to:\n"));
    for (U8 i = 0; i < 8; i++) {
        LOG(STRING("PAT "));
        LOG(i);
        LOG(STRING(": "));
        LOG(patEncodingToString[patValues.pats[i].pat], NEWLINE);
    }
}
