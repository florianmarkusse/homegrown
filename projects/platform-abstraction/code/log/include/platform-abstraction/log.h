#ifndef PLATFORM_ABSTRACTION_LOG_H
#define PLATFORM_ABSTRACTION_LOG_H

#ifdef FREESTANDING_ENVIRONMENT
#include "log/log.h"
#else
#include "posix/log/log.h"
#endif

#endif
