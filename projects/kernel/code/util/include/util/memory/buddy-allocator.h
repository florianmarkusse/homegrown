#ifndef UTIL_MEMORY_BUDDY_ALLOCATOR_H
#define UTIL_MEMORY_BUDDY_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

typedef struct {
    uint64_t size;
    bool isFree;
} BuddyBlock;

__attribute((unused)) static inline BuddyBlock *nextBuddy(BuddyBlock *block) {
    return (BuddyBlock *)((char *)block + block->size);
}

BuddyBlock *splitBuddy(BuddyBlock *block, uint64_t size);

BuddyBlock *findBestBuddy(BuddyBlock *head, BuddyBlock *tail, uint64_t size);

typedef struct {
    BuddyBlock *head;
    BuddyBlock *tail;

    void **jmp_buf;
} BuddyAllocator;

BuddyAllocator createBuddyAllocator(char *data, uint64_t size);

void coalesceBuddies(BuddyBlock *head, BuddyBlock *tail);

__attribute((unused, malloc)) void *buddyAlloc(BuddyAllocator *buddyAllocator,
                                               int64_t size, int64_t count,
                                               unsigned char flags);

void freeBuddy(BuddyAllocator *buddyAllocator, void *data);

#ifdef __cplusplus
}
#endif

#endif
