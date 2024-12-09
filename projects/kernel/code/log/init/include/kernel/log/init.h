#ifndef KERNEL_LOG_INIT_H
#define KERNEL_LOG_INIT_H

#include "shared/memory/allocator/arena.h"
#include "shared/types/array-types.h"

extern U8_max_a flushBuf;

void initLogger(Arena *perm);

#endif
