#ifndef UTIL_MEMORY_BUDDY_ALLOCATOR_H
#define UTIL_MEMORY_BUDDY_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/types.h"

typedef struct {
    uint64_t size;
    bool isFree;
} flo_BuddyBlock;

__attribute((unused)) static inline flo_BuddyBlock *
flo_nextBuddy(flo_BuddyBlock *block) {
    return (flo_BuddyBlock *)((char *)block + block->size);
}

flo_BuddyBlock *flo_splitBuddy(flo_BuddyBlock *block, uint64_t size);

flo_BuddyBlock *flo_findBestBuddy(flo_BuddyBlock *head, flo_BuddyBlock *tail,
                                  uint64_t size);

typedef struct {
    flo_BuddyBlock *head;
    flo_BuddyBlock *tail;

    void **jmp_buf;
} flo_BuddyAllocator;

flo_BuddyAllocator flo_createBuddyAllocator(char *data, uint64_t size);

void flo_coalesceBuddies(flo_BuddyBlock *head, flo_BuddyBlock *tail);

__attribute((unused, malloc, alloc_size(2, 3))) void *
flo_buddyAlloc(flo_BuddyAllocator *buddyAllocator, int64_t size, int64_t count,
               unsigned char flags);

void flo_freeBuddy(flo_BuddyAllocator *buddyAllocator, void *data);

#ifdef __cplusplus
}
#endif

#endif
