#ifndef PLATFORM_ABSTRACTION_INTERRUPTS_H
#define PLATFORM_ABSTRACTION_INTERRUPTS_H

// TODO: Change to just use non-x86 specific interrupt stuff
#ifdef X86_ARCHITECTURE

#include "x86/fault.h"

void initIDT();
__attribute__((noreturn)) void triggerFault(Fault fault);

#else
#error "Could not match ARCHITECTURE"
#endif

#endif
