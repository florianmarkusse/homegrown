#include "x86/cpu/status/test.h"
#include "platform-abstraction/log.h"
#include "shared/text/string.h"
#include "x86/cpu/status/core.h"

void appendExpectedInterrupt(Fault fault) {
    KLOG(STRING("Missing interrupt\n"));
    appendInterrupt(fault);
    KLOG(STRING("\n"));
}

void appendInterrupts(bool *expectedFaults, bool *actualFaults) {
    KLOG(STRING("Interrupts Table\n"));
    for (U64 i = 0; i < CPU_FAULT_COUNT; i++) {
        appendInterrupt(i);
        KLOG(STRING("\tExpected: "));
        KLOG(stringWithMinSizeDefault(
            expectedFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        KLOG(STRING("\tActual: "));
        KLOG(stringWithMinSizeDefault(
            actualFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        if (expectedFaults[i] != actualFaults[i]) {
            KLOG(STRING("\t!!!"));
        }
        KLOG(STRING("\n"));
    }
}
