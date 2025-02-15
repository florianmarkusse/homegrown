#include "platform-abstraction/memory/management/status.h"

#include "platform-abstraction/log.h"
#include "platform-abstraction/physical/status.h"
#include "platform-abstraction/virtual/status.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "x86-policy/virtual.h"
#include "x86-virtual.h"
#include "x86/memory/definitions.h"
#include "x86/memory/pat.h"

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
