#ifndef PLATFORM_ABSTRACTION_CPU_H
#define PLATFORM_ABSTRACTION_CPU_H

#ifdef FREESTANDING_ENVIRONMENT
#include "x86/cpu/real/cpu.h"
#elif POSIX_ENVIRONMENT
#include "x86/cpu/mock/cpu.h"
#else
#error "Could not match ENVIRONMENT"
#endif

#endif
