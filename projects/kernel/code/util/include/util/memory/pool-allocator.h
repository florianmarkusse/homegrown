#ifndef UTIL_MEMORY_POOL_ALLOCATOR_H
#define UTIL_MEMORY_POOL_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

struct PoolHead {
    struct PoolHead *next;
};
typedef struct PoolHead PoolHead;

typedef struct {
    char *beg;
    I64 cap;
    I64 chunkSize;

    PoolHead *head;

    void **jmp_buf;
} PoolAllocator;

void freePool(PoolAllocator *pool);

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
PoolAllocator createPoolAllocator(char *buffer, I64 cap,
                                          I64 chunkSize);

__attribute((malloc)) void *poolAlloc(PoolAllocator *pool,
                                          unsigned char flags);

void freePoolNode(PoolAllocator *pool, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
