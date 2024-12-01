#include "x86/cpu/status/test.h"
#include "platform-abstraction/log.h"
#include "x86/cpu/status/core.h"
#include "shared/text/string.h"


void appendExpectedInterrupt(Fault fault) {
    LOG(STRING("Missing interrupt\n"));
    appendInterrupt(fault);
    LOG(STRING("\n"));
}

void appendInterrupts(bool *expectedFaults, bool *actualFaults) {
    LOG(STRING("Interrupts Table\n"));
    for (U64 i = 0; i < FAULT_NUMS; i++) {
        appendInterrupt(i);
        LOG(STRING("\tExpected: "));
        LOG(stringWithMinSizeDefault(
            expectedFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        LOG(STRING("\tActual: "));
        LOG(stringWithMinSizeDefault(
            actualFaults[i] ? STRING("ON") : STRING("OFF"), 3));
        if (expectedFaults[i] != actualFaults[i]) {
            LOG(STRING("\t!!!"));
        }
        LOG(STRING("\n"));
    }
}
