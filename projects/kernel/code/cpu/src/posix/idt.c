#include "cpu/idt.h"

static bool triggeredFaults[FAULT_NUMS];

static void **interruptJumper;

void initIDT() {}

void triggerFault(Fault fault) {
    triggeredFaults[fault] = true;
    __builtin_longjmp(interruptJumper, 1);
}

void initIDTTest(void *long_jmp[5]) { interruptJumper = long_jmp; }

bool *getTriggeredFaults() { return triggeredFaults; }
