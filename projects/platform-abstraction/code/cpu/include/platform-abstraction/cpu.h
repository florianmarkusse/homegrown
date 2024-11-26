#ifndef PLATFORM_ABSTRACTION_CPU_H
#define PLATFORM_ABSTRACTION_CPU_H

#ifdef FREESTANDING_BUILD
#include "x86/cpu/real/cpu.h"
#else
#include "x86/cpu/real/mock.h"
#endif

#endif
