#include "cpu/idt.h"

static bool triggeredFaults[FAULT_NUMS];

void setupIDT() {}

void triggerFault(Fault fault) { triggeredFaults[fault] = true; }
bool isFaultTriggered(Fault fault) { return triggeredFaults[fault]; }
