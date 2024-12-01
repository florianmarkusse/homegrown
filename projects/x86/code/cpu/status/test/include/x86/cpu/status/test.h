#ifndef X86_CPU_STATUS_TEST_H
#define X86_CPU_STATUS_TEST_H

#include "platform-abstraction/idt.h"

void appendExpectedInterrupt(Fault fault);
void appendInterrupts(bool *expectedFaults, bool *actualFaults);

#endif
