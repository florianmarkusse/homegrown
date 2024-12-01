#ifndef STATUS_X86_CPU_CORE_H
#define STATUS_X86_CPU_CORE_H

#include "platform-abstraction/idt.h"

void appendInterrupt(Fault fault);

#endif
