#include "abstraction/interrupts.h"
#include "x86/fault.h"
#include "x86/idt.h"

void interruptNoMorePhysicalMemory() {
    triggerFault(FAULT_NO_MORE_PHYSICAL_MEMORY);
}
void interruptTooLargeAllocation() { triggerFault(FAULT_TOO_LARGE_ALLOCATION); }
