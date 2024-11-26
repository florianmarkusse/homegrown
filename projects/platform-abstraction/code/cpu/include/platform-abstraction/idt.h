#ifndef PLATFORM_ABSTRACTION_IDT_H
#define PLATFORM_ABSTRACTION_IDT_H

#ifdef FREESTANDING_BUILD
#include "x86/cpu/real/idt.h"
#else
#include "x86/cpu/real/idt.h"
#endif

#endif
