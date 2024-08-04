#ifndef UTIL_MEMORY_BUDDY_ALLOCATOR_H
#define UTIL_MEMORY_BUDDY_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"

typedef struct {
    U64 size;
    bool isFree;
} BuddyBlock;

__attribute((unused)) static inline BuddyBlock *nextBuddy(BuddyBlock *block) {
    return (BuddyBlock *)((I8 *)block + block->size);
}

BuddyBlock *splitBuddy(BuddyBlock *block, U64 size);

BuddyBlock *findBestBuddy(BuddyBlock *head, BuddyBlock *tail, U64 size);

typedef struct {
    BuddyBlock *head;
    BuddyBlock *tail;

    void **jmp_buf;
} BuddyAllocator;

BuddyAllocator createBuddyAllocator(I8 *data, U64 size);

void coalesceBuddies(BuddyBlock *head, BuddyBlock *tail);

__attribute((unused, malloc)) void *buddyAlloc(BuddyAllocator *buddyAllocator,
                                               I64 size, I64 count,
                                               U8 flags);

void freeBuddy(BuddyAllocator *buddyAllocator, void *data);

#ifdef __cplusplus
}
#endif

#endif
