#include "util/memory/pool-allocator.h"
#include "util/assert.h"        // for FLO_ASSERT
#include "util/memory/macros.h" // for FLO_NULL_ON_FAIL, FLO_ZERO_MEMORY
#include "util/memory/memory.h"

void flo_freePool(flo_PoolAllocator *pool) {
    uint64_t chunkCount = pool->cap / pool->chunkSize;
    uint64_t i;

    for (i = 0; i < chunkCount; i++) {
        void *ptr = &pool->beg[i * pool->chunkSize];
        flo_PoolHead *node = (flo_PoolHead *)ptr;
        node->next = pool->head;
        pool->head = node;
    }
}

/*
 * Set up the pool allocator values, except for the jmp_buf!
 */
flo_PoolAllocator flo_createPoolAllocator(char *buffer, int64_t cap,
                                          int64_t chunkSize) {
    FLO_ASSERT(cap > 0);
    FLO_ASSERT((cap & (cap - 1)) == 0);

    FLO_ASSERT(chunkSize > 0);
    FLO_ASSERT((chunkSize & (chunkSize - 1)) == 0);
    FLO_ASSERT(chunkSize > FLO_SIZEOF(flo_PoolHead));

    FLO_ASSERT(cap > chunkSize);

    flo_PoolAllocator result;

    result.beg = buffer;
    result.cap = cap;
    result.chunkSize = chunkSize;

    result.head = NULL;

    flo_freePool(&result);

    return result;
}

__attribute((malloc)) void *flo_poolAlloc(flo_PoolAllocator *pool,
                                          unsigned char flags) {
    flo_PoolHead *node = pool->head;

    if (node == NULL) {
        FLO_ASSERT(false);
        if (flags & FLO_NULL_ON_FAIL) {
            return NULL;
        }
        __builtin_longjmp(pool->jmp_buf, 1);
    }

    pool->head = pool->head->next;

    return flags & FLO_ZERO_MEMORY ? memset(node, 0, pool->chunkSize) : node;
}

void flo_freePoolNode(flo_PoolAllocator *pool, void *ptr) {
    FLO_ASSERT((void *)pool->beg <= ptr &&
               ptr < (void *)(pool->beg + pool->cap));

    flo_PoolHead *node = (flo_PoolHead *)ptr;
    node->next = pool->head;
    pool->head = node;
}
