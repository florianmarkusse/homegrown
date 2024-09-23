#ifndef STATUS_IDT_STATUS_H
#define STATUS_IDT_STATUS_H

#include "cpu/idt.h"

void appendInterrupt(Fault fault);
void appendExpectedInterrupt(Fault fault);
void appendInterrupts(bool *expectedFaults);

#endif
