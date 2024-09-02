#include "memory/management/virtual.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"
#include "memory/management/definitions.h"
#include "memory/management/physical.h"
#include "memory/manipulation/manipulation.h"
#include "util/assert.h"
#include "util/macros.h"
#include "util/maths.h"

/* NOLINTNEXTLINE */
typedef struct VirtualPageTable;
typedef struct {
    U64 pages[PAGE_FRAME_SIZE];
} VirtualPageTable;

static VirtualPageTable *level4PageTable;

U64 getZeroBasePage() {
    PagedMemory memoryForAddresses[1];
    PagedMemory_a memory =
        allocPhysicalPages((PagedMemory_a){.buf = memoryForAddresses,
                                           .len = COUNTOF(memoryForAddresses)},
                           BASE_PAGE);
    memset(memory.buf, 0, PAGE_FRAME_SIZE);
    return (U64)memory.buf;
}

// Should set this value to the kernel memory instead.
void initVirtualMemoryManager() {
    level4PageTable = (VirtualPageTable *)getZeroBasePage();
}

void mapMemoryAt(U64 phys, U64 virt, U64 size, U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0x0000 || ((virt) >> 48L) == 0xFFFF);

    U64 end = virt + size;
    U64 *pageEntry = NULL;

    virt = ALIGN_DOWN_EXP(virt, PAGE_FRAME_SHIFT);
    phys = ALIGN_DOWN_EXP(phys, PAGE_FRAME_SHIFT);
    /* walk the page tables and add the missing pieces */
    for (; virt < end; virt += PAGE_FRAME_SIZE, phys += PAGE_FRAME_SIZE) {
        /* 512G */
        pageEntry = &((
            (U64 *)level4PageTable)[(virt >> LEVEL_4_SHIFT) & PAGE_TABLE_MASK]);
        if (!*pageEntry) {
            *pageEntry = (getZeroBasePage() | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 1G */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> LEVEL_3_SHIFT) & PAGE_TABLE_MASK]);
        if (!*pageEntry) {
            *pageEntry = (getZeroBasePage() | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 2M  */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> LEVEL_2_SHIFT) & PAGE_TABLE_MASK]);
        if (!*pageEntry) {
            *pageEntry = (getZeroBasePage() | (PAGE_PRESENT | PAGE_WRITABLE));
        }
        /* 4K */
        pageEntry = (U64 *)(*pageEntry & ~(PAGE_MASK));
        pageEntry = &(pageEntry[(virt >> LEVEL_1_SHIFT) & PAGE_TABLE_MASK]);

        /* if this page is already mapped, that means the kernel has invalid,
         * overlapping segments */
        ASSERT(!*pageEntry);

        *pageEntry = phys | PAGE_PRESENT | PAGE_WRITABLE | additionalFlags;
    }
}

VirtualPageTable *descend(VirtualPageTable *table, U64 virtual, U64 shift) {
    VirtualPageTable *lowerEntry = NULL;
    lowerEntry =
        (VirtualPageTable *)
            table->pages[ALIGN_DOWN_EXP((virtual >> shift), PAGE_FRAME_SHIFT)];
    return lowerEntry;
}

// The caller should take care that the virtual address and physical address
// are correctly aligned. If they are not, not sure what the caller wanted
// to accomplish.
void mapMemoryNew(U64 virtual, PagedMemory memory, PageType pageType,
                  U64 additionalFlags) {
    // 256 TiB in total
    // 512 GiB per entry
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0x0000 || ((virt) >> 48L) == 0xFFFF);

    U64 pageSize = pageTypeToPageSize[pageType];

    U64 virtualEnd = virtual + pageSize * memory.numberOfPages;

    // 512 GiB in total
    // 1 GiB per entry, hence this table can contain huge table entries
    U64 level4Index =
        ALIGN_DOWN_EXP(virtual >> LEVEL_4_SHIFT, PAGE_FRAME_SHIFT);
    VirtualPageTable *level3PageTable =
        descend(level4PageTable, virtual, LEVEL_4_SHIFT);
    if (!level3PageTable) {
        level3PageTable = (VirtualPageTable *)(getZeroBasePage() |
                                               (PAGE_PRESENT | PAGE_WRITABLE));
    }

    // 1 GiB in total
    // 2 Mib per entry, hence this table can contain large table entries
    VirtualPageTable *level2PageTable =
        descend(level3PageTable, virtual, LEVEL_3_SHIFT);
    if (!level2PageTable) {
        level2PageTable = (VirtualPageTable *)(getZeroBasePage() |
                                               (PAGE_PRESENT | PAGE_WRITABLE));
    }

    // 2MiB in total
    // 4Kib per entry
    VirtualPageTable *level1PageTable =
        descend(level2PageTable, virtual, LEVEL_2_SHIFT);
    if (!level1PageTable) {
        level1PageTable = (VirtualPageTable *)(getZeroBasePage() |
                                               (PAGE_PRESENT | PAGE_WRITABLE));
    }

    // 4KiB in total
    U64 *basePageFrame =
        (U64 *)descend(level1PageTable, virtual, LEVEL_1_SHIFT);

    if (!*basePageFrame) {
        // There  is already something mapped, need to invalidate cache...
    }
    *basePageFrame = (memory.pageStart | (PAGE_PRESENT | PAGE_WRITABLE));
}
