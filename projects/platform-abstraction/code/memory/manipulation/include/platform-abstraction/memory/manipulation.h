#ifndef PLATFORM_ABSTRACTION_MEMORY_MANIPULATION_H
#define PLATFORM_ABSTRACTION_MEMORY_MANIPULATION_H

#ifdef FREESTANDING_ENVIRONMENT
#include "kernel/memory/manipulation.h"
#elif POSIX_ENVIRONMENT
#include "posix/memory/manipulation.h"
#else
#error "Could not match ENVIRONMENT"
#endif

#endif
