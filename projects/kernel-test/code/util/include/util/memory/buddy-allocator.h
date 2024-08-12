#ifndef UTIL_MEMORY_BUDDY_ALLOCATOR_H
#define UTIL_MEMORY_BUDDY_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    size_t size;
    bool isFree;
} BuddyBlock;

__attribute((unused)) static inline BuddyBlock *
nextBuddy(BuddyBlock *block) {
    return (BuddyBlock *)((char *)block + block->size);
}

BuddyBlock *splitBuddy(BuddyBlock *block, size_t size);

BuddyBlock *findBestBuddy(BuddyBlock *head, BuddyBlock *tail,
                                  size_t size);

typedef struct {
    BuddyBlock *head;
    BuddyBlock *tail;

    void **jmp_buf;
} BuddyAllocator;

BuddyAllocator createBuddyAllocator(char *data, size_t size);

void coalesceBuddies(BuddyBlock *head, BuddyBlock *tail);

__attribute((unused, malloc)) void *
buddyAlloc(BuddyAllocator *buddyAllocator, ptrdiff_t size,
               ptrdiff_t count, unsigned char flags);

void freeBuddy(BuddyAllocator *buddyAllocator, void *data);

#ifdef __cplusplus
}
#endif

#endif
