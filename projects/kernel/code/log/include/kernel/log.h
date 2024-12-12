#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"

void appendToFlushBuffer(string data, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

#endif
