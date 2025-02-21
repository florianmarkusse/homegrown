#ifndef X86_IDT_H
#define X86_IDT_H

#include "x86/fault.h"

__attribute__((noreturn)) void triggerFault(Fault fault);

#endif
