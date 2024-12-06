#ifndef PLATFORM_ABSTRACTION_IDT_H
#define PLATFORM_ABSTRACTION_IDT_H

#ifdef X86_ARCHITECTURE
#ifdef FREESTANDING_ENVIRONMENT
#include "x86/cpu/real/idt.h"
#elif POSIX_ENVIRONMENT
#include "x86/cpu/mock/idt.h"
#else
#error "Could not match ENVIRONMENT"
#endif
#else
#error "Could not match ARCHITECTURE"
#endif

#endif
