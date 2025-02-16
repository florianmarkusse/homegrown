#ifndef PLATFORM_ABSTRACTION_INTERRUPTS_H
#define PLATFORM_ABSTRACTION_INTERRUPTS_H

void initIDT();

__attribute__((noreturn)) void interruptNoMorePhysicalMemory();
__attribute__((noreturn)) void interruptTooLargeAllocation();

#endif
