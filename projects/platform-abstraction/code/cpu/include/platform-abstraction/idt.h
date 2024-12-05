#ifndef PLATFORM_ABSTRACTION_IDT_H
#define PLATFORM_ABSTRACTION_IDT_H

#ifdef FREESTANDING_ENVIRONMENT
#include "x86/cpu/real/idt.h"
#else
#include "x86/cpu/mock/idt.h"
#endif

#endif
