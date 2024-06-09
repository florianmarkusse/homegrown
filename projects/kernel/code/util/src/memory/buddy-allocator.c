#include "util/memory/buddy-allocator.h"
#include "util/assert.h"        // for ASSERT
#include "util/memory/macros.h" // for SIZEOF, NULL_ON_FAIL, ZE...
#include "util/memory/memory.h"

BuddyBlock *splitBuddy(BuddyBlock *block, uint64_t size) {
    ASSERT(size > 0);

    if (block != NULL) {
        while (size * 2 < block->size) {
            uint64_t halfSize = block->size >> 1;
            block->size = halfSize;
            block = nextBuddy(block);
            block->size = halfSize;
            block->isFree = true;
        }

        if (size <= block->size) {
            return block;
        }
    }

    return NULL;
}

BuddyBlock *findBestBuddy(BuddyBlock *head, BuddyBlock *tail, uint64_t size) {
    ASSERT(size > 0);

    BuddyBlock *bestBlock = NULL;
    BuddyBlock *block = head;             // Left Buddy
    BuddyBlock *buddy = nextBuddy(block); // Right Buddy

    // The entire memory section between head and tail is free,
    // just call 'buddy_block_split' to get the allocation
    if (buddy == tail && block->isFree) {
        return splitBuddy(block, size);
    }

    // Find the block which is the 'bestBlock' to requested allocation sized
    while (block < tail &&
           buddy < tail) { // make sure the buddies are within the range
        // If both buddies are free, coalesce them together
        // NOTE: this is an optimization to reduce fragmentation
        //       this could be completely ignored
        if (block->isFree && buddy->isFree && block->size == buddy->size) {
            block->size <<= 1;
            if (size <= block->size &&
                (bestBlock == NULL || block->size <= bestBlock->size)) {
                bestBlock = block;
            }

            block = nextBuddy(buddy);
            if (block < tail) {
                // Delay the buddy block for the next iteration
                buddy = nextBuddy(block);
            }
            continue;
        }

        if (block->isFree && size <= block->size &&
            (bestBlock == NULL || block->size < bestBlock->size)) {
            bestBlock = block;
        }

        if (buddy->isFree && size <= buddy->size &&
            (bestBlock == NULL || buddy->size < bestBlock->size)) {
            // If each buddy are the same size, then it makes more sense
            // to pick the buddy as it "bounces around" less
            bestBlock = buddy;
        }

        if (block->size < buddy->size) {
            block = nextBuddy(buddy);
            if (block < tail) {
                // Delay the buddy block for the next iteration
                buddy = nextBuddy(block);
            }
        } else {
            // Buddy was split into smaller blocks
            block = buddy;
            buddy = nextBuddy(buddy);
        }
    }

    if (bestBlock != NULL) {
        // This will handle the case if the 'best_block' is also the perfect fit
        return splitBuddy(bestBlock, size);
    }

    // Maybe out of memory
    return NULL;
}

BuddyAllocator createBuddyAllocator(char *data, uint64_t size) {
    ASSERT(size > 0);
    ASSERT((size & (size - 1)) == 0);
    ASSERT(size > (uint64_t)sizeof(BuddyBlock));

    BuddyAllocator result;

    result.head = (BuddyBlock *)data;
    result.head->size = size;
    result.head->isFree = true;

    result.tail = nextBuddy(result.head);

    return result;
}

/**
 * Coalescing algorithm. Think of left and right of 2 nodes of a binary tree.
 * You can only merge nodes that are actually part of the same branch.
 */
void coalesceBuddies(BuddyBlock *head, BuddyBlock *tail) {
    while (true) {
        // Keep looping until there are no more buddies to coalesce

        BuddyBlock *block = head;
        BuddyBlock *buddy = nextBuddy(block);

        bool noCoalesce = true;
        while (block < tail &&
               buddy < tail) { // make sure the buddies are within the range
            if (block->size < buddy->size) {
                block = nextBuddy(buddy);
                if (block < tail) {
                    // Leave the buddy block for the next iteration
                    buddy = nextBuddy(block);
                }
            } else if (block->size > buddy->size) {
                // The buddy block is split into smaller blocks
                block = buddy;
                buddy = nextBuddy(buddy);
            } else {
                if (block->isFree && buddy->isFree) {
                    // Coalesce buddies into one
                    block->size <<= 1;
                    block = nextBuddy(block);
                    if (block < tail) {
                        buddy = nextBuddy(block);
                        noCoalesce = false;
                    }
                } else {
                    block = nextBuddy(buddy);
                    if (block < tail) {
                        // Leave the buddy block for the next iteration
                        buddy = nextBuddy(block);
                    }
                }
            }
        }

        if (noCoalesce) {
            return;
        }
    }
}

__attribute((unused, malloc)) void *buddyAlloc(BuddyAllocator *buddyAllocator,
                                               int64_t size, int64_t count,
                                               unsigned char flags) {
    ASSERT(size > 0);

    uint64_t total = size * count;
    uint64_t totalSize = total + SIZEOF(BuddyBlock);

    BuddyBlock *found =
        findBestBuddy(buddyAllocator->head, buddyAllocator->tail, totalSize);
    if (found == NULL) {
        // Try to coalesce all the free buddy blocks and then search again
        coalesceBuddies(buddyAllocator->head, buddyAllocator->tail);
        found = findBestBuddy(buddyAllocator->head, buddyAllocator->tail,
                              totalSize);
    }

    if (found != NULL) {
        found->isFree = false;
        void *result = (void *)((char *)found + SIZEOF(BuddyBlock));
        if (ZERO_MEMORY & flags) {
            memset(result, 0, total);
        }
        return result;
    }

    ASSERT(false);
    if (flags & NULL_ON_FAIL) {
        return NULL;
    }
    __builtin_longjmp(buddyAllocator->jmp_buf, 1);
}

void freeBuddy(BuddyAllocator *buddyAllocator, void *data) {
    ASSERT((void *)buddyAllocator->head <= data);
    ASSERT(data < (void *)buddyAllocator->tail);

    BuddyBlock *block = (BuddyBlock *)((char *)data - SIZEOF(BuddyBlock));
    block->isFree = true;

    coalesceBuddies(buddyAllocator->head, buddyAllocator->tail);
}
