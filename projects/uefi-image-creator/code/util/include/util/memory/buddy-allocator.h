#ifndef UTIL_MEMORY_BUDDY_ALLOCATOR_H
#define UTIL_MEMORY_BUDDY_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct {
    size_t size;
    bool isFree;
} flo_BuddyBlock;

__attribute((unused)) static inline flo_BuddyBlock *
flo_nextBuddy(flo_BuddyBlock *block) {
    return (flo_BuddyBlock *)((char *)block + block->size);
}

flo_BuddyBlock *flo_splitBuddy(flo_BuddyBlock *block, size_t size);

flo_BuddyBlock *flo_findBestBuddy(flo_BuddyBlock *head, flo_BuddyBlock *tail,
                                  size_t size);

typedef struct {
    flo_BuddyBlock *head;
    flo_BuddyBlock *tail;

    void **jmp_buf;
} flo_BuddyAllocator;

flo_BuddyAllocator flo_createBuddyAllocator(char *data, size_t size);

void flo_coalesceBuddies(flo_BuddyBlock *head, flo_BuddyBlock *tail);

__attribute((unused, malloc)) void *
flo_buddyAlloc(flo_BuddyAllocator *buddyAllocator, ptrdiff_t size,
               ptrdiff_t count, unsigned char flags);

void flo_freeBuddy(flo_BuddyAllocator *buddyAllocator, void *data);

#ifdef __cplusplus
}
#endif

#endif
