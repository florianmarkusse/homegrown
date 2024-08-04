#include "memory/definitions.h"
#include "types.h"
#include "util/assert.h"

static U64 *level4PageTable;

U64 allocAndZero(U64 pages) { return 0; }

// TODO: refactor to use ring buffer Range operators instead of hardcoding the
// ands with a calculated mask.
void mapMemoryAt(U64 phys, U64 virt, U64 size) {
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0x0000 || ((virt) >> 48L) == 0xFFFF);

    U64 end = virt + size;
    U64 *pageEntry = NULL;
    /* walk the page tables and add the missing pieces */
    for (virt &= ~(PAGE_MASK), phys &= ~(PAGE_MASK); virt < end;
         virt += PAGE_SIZE, phys += PAGE_SIZE) {
        /* 512G */
        pageEntry =
            &(((U64 *)level4PageTable)[(virt >> 39L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            U64 addr = allocAndZero(1);
            *pageEntry = (addr | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 1G */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 30L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 2M  */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 21L) & PAGE_ENTRY_MASK]);
        if (!*pageEntry) {
            *pageEntry = (allocAndZero(1) | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 4K */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> 12L) & PAGE_ENTRY_MASK]);
        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        if (!*pageEntry) {
            *pageEntry = phys | (PAGE_PRESENT | PAGE_WRITABLE);
        }
    }
}
