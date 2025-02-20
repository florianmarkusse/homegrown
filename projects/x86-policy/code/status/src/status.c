#include "abstraction/memory/management/status.h"

#include "abstraction/log.h"
#include "abstraction/memory/physical/status.h"
#include "abstraction/memory/virtual/status.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "x86-policy/virtual.h"

static void appendVirtualRegionStatus(VirtualRegion region) {
    KLOG(STRING("Start: "));
    KLOG((void *)region.start, NEWLINE);
    KLOG(STRING("End: "));
    KLOG((void *)region.end, NEWLINE);
}

void appendMemoryManagementStatus() {
    KLOG(STRING("Available Virtual Memory\n"));
    KLOG(STRING("Lower half (0x0000_000000000000):\n"));
    appendVirtualRegionStatus(lowerHalfRegion);
    KLOG(STRING("Higher half(0xFFFF_000000000000):\n"));
    appendVirtualRegionStatus(higherHalfRegion);

    appendVirtualMemoryManagerStatus();
    appendPhysicalMemoryManagerStatus();
}
