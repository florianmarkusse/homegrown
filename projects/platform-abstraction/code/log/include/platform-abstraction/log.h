#ifndef PLATFORM_ABSTRACTION_LOG_H
#define PLATFORM_ABSTRACTION_LOG_H

#ifdef FREESTANDING_ENVIRONMENT
#include "log/log.h"
#elif POSIX_ENVIRONMENT
#include "posix/log/log.h"
#else
#error "Could not match ENVIRONMENT"
#endif

#endif
