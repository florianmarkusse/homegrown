#ifndef X86_CPU_STATUS_CORE_H
#define X86_CPU_STATUS_CORE_H

#include "platform-abstraction/idt.h"

void appendInterrupt(Fault fault);

#endif
