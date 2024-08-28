#include "cpu/idt.h"
#include "interoperation/types.h"
#include "log/log.h"

static bool triggeredFaults[FAULT_NUMS];

static void **interruptJumper;

void initIDT() {}

void triggerFault(Fault fault) {
    triggeredFaults[fault] = true;
    __builtin_longjmp(interruptJumper, 1);
}

void initIDTTest(void *long_jmp[5]) { interruptJumper = long_jmp; }

bool *getTriggeredFaults() { return triggeredFaults; }
void resetTriggeredFaults() {
    for (U64 i = 0; i < FAULT_NUMS; i++) {
        triggeredFaults[i] = false;
    }
}

bool compareInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();

    for (U64 i = 0; i < FAULT_NUMS; i++) {
        if (expectedFaults[i] != actualFaults[i]) {
            return false;
        }
    }

    return true;
}

void appendInterrupt(Fault fault) {
    LOG(STRING("Fault #: "));
    LOG(fault);
    LOG(STRING("\tMsg: "));
    LOG(stringWithMinSizeDefault(faultToString(fault), 30));
}

void appendExpectedInterrupt(Fault fault) {
    LOG(STRING("Missing interrupt\n"));
    appendInterrupt(fault);
    LOG(STRING("\n"));
}

void appendInterrupts(bool *expectedFaults) {
    bool *actualFaults = getTriggeredFaults();
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
