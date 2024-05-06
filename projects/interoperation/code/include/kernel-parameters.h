#ifndef KERNEL_PARAMETERS_H
#define KERNEL_PARAMETERS_H

typedef __INT8_TYPE__ I8;
typedef __UINT8_TYPE__ U8;
typedef __INT16_TYPE__ I16;
typedef __UINT16_TYPE__ U16;
typedef __INT32_TYPE__ I32;
typedef __UINT32_TYPE__ U32;
typedef __INT64_TYPE__ I64;
typedef __UINT64_TYPE__ U64;

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
