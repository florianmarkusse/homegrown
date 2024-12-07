#ifndef PLATFORM_ABSTRACTION_LOG_H
#define PLATFORM_ABSTRACTION_LOG_H

#ifdef FREESTANDING_ENVIRONMENT
#include "kernel/log.h"
#elif POSIX_ENVIRONMENT
#include "posix/log.h"
#else
#error "Could not match ENVIRONMENT"
#endif

#endif
