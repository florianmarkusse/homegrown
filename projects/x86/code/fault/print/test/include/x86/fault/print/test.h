#ifndef X86_FAULT_PRINT_TEST_H
#define X86_FAULT_PRINT_TEST_H

#include "x86/fault.h"

void appendExpectedInterrupt(Fault fault);
void appendInterrupts(bool *expectedFaults, bool *actualFaults);

#endif
