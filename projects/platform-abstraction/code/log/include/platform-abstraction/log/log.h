#ifndef PLATFORM_ABSTRACTION_LOG_LOG_H
#define PLATFORM_ABSTRACTION_LOG_LOG_H

#include "shared/log/log.h"

#ifdef FREESTANDING_BUILD
#include "log/log.h"
#else
#include "posix/log/log.h"
#endif

#endif
