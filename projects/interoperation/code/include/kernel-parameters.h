#ifndef KERNEL_PARAMETERS_H
#define KERNEL_PARAMETERS_H

#include "types.h"
typedef struct {
    U64 ptr;
    U64 size;
    U32 columns;
    U32 rows;
    U32 scanline;
} FrameBuffer;

typedef struct {
    U64 ptr;
    U64 size;
} MemoryMap;

typedef struct {
    FrameBuffer fb;
    MemoryMap *memory;
} KernelParameters;

#endif
