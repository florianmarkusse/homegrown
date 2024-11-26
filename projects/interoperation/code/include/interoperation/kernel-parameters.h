#ifndef KERNEL_PARAMETERS_H
#define KERNEL_PARAMETERS_H

#include "interoperation/memory/descriptor.h"
#include "shared/types/types.h"
typedef struct {
    U64 ptr;
    U64 size;
    U32 columns;
    U32 rows;
    U32 scanline;
} FrameBuffer;

typedef struct {
    USize totalDescriptorSize;
    MemoryDescriptor *descriptors;
    USize descriptorSize;
} KernelMemory;

typedef struct {
    FrameBuffer fb;
    KernelMemory memory;
    U64 level4PageTable;

} KernelParameters;

#endif
