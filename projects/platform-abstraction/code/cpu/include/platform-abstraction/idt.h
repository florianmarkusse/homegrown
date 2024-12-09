#ifndef PLATFORM_ABSTRACTION_IDT_H
#define PLATFORM_ABSTRACTION_IDT_H

#ifdef X86_ARCHITECTURE

#include "x86/cpu/fault.h"

void initIDT();
__attribute__((noreturn)) void triggerFault(Fault fault);

#else
#error "Could not match ARCHITECTURE"
#endif

#endif
