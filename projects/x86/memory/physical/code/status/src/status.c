#include "abstraction/memory/physical/status.h"

#include "abstraction/log.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86/memory/physical.h"

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
