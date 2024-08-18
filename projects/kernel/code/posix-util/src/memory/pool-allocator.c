#include "util/memory/pool-allocator.h"
#include "util/assert.h"        // for ASSERT
#include "util/memory/macros.h" // for NULL_ON_FAIL, ZERO_MEMORY
#include <string.h>             // for memset

void freePool(PoolAllocator *pool) {
    size_t chunkCount = pool->cap / pool->chunkSize;
    size_t i;

    for (i = 0; i < chunkCount; i++) {
        void *ptr = &pool->beg[i * pool->chunkSize];
        PoolHead *node = (PoolHead *)ptr;
        node->next = pool->head;
        pool->head = node;
    }
}

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
PoolAllocator createPoolAllocator(char *buffer, ptrdiff_t cap,
                                          ptrdiff_t chunkSize) {
    ASSERT(cap > 0);
    ASSERT((cap & (cap - 1)) == 0);

    ASSERT(chunkSize > 0);
    ASSERT((chunkSize & (chunkSize - 1)) == 0);
    ASSERT(chunkSize > SIZEOF(PoolHead));

    ASSERT(cap > chunkSize);

    PoolAllocator result;

    result.beg = buffer;
    result.cap = cap;
    result.chunkSize = chunkSize;

    result.head = NULL;

    freePool(&result);

    return result;
}

__attribute((malloc)) void *poolAlloc(PoolAllocator *pool,
                                          unsigned char flags) {
    PoolHead *node = pool->head;

    if (node == NULL) {
        ASSERT(false);
        if (flags & NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(pool->jmp_buf, 1);
    }

    pool->head = pool->head->next;

    return flags & ZERO_MEMORY ? memset(node, 0, pool->chunkSize) : node;
}

void freePoolNode(PoolAllocator *pool, void *ptr) {
    ASSERT((void *)pool->beg <= ptr &&
               ptr < (void *)(pool->beg + pool->cap));

    PoolHead *node = (PoolHead *)ptr;
    node->next = pool->head;
    pool->head = node;
}