#include "x86/cpu/mock/idt.h"
#include "shared/types/types.h"

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
