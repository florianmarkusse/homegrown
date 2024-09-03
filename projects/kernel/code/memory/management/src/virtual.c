#include "memory/management/virtual.h"
#include "cpu/x86.h"
#include "interoperation/memory/definitions.h"
#include "interoperation/types.h"
#include "log/log.h"
#include "memory/management/definitions.h"
#include "memory/management/physical.h"
#include "memory/manipulation/manipulation.h"
#include "util/assert.h"
#include "util/macros.h"
#include "util/maths.h"

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

typedef struct {
    U8 pat : 3;
    U8 reserved : 5;
} PATEntry;
typedef struct {
    union {
        PATEntry pats[8];
        U64 value;
    };
} PAT;

// Should set this value to the kernel memory instead.
void initVirtualMemoryManager(U64 level4Address) {
    FLUSH_AFTER {
        LOG(STRING("The address is: "));
        LOG(level4Address, NEWLINE);
    }

    level4PageTable = (VirtualPageTable *)level4Address;

    PAT patValues = {.value = rdmsr(0x277)};

    FLUSH_AFTER {
        for (U8 i = 0; i < 8; i++) {
            LOG(STRING("Pat "));
            LOG(i);
            LOG(STRING(": "));
            LOG(patValues.pats[i].pat, NEWLINE);
        }
    }

    patValues.pats[0].pat = 0b001;

    wrmsr(0x277, patValues.value);

    patValues.value = rdmsr(0x277);

    FLUSH_AFTER {
        for (U8 i = 0; i < 8; i++) {
            LOG(STRING("Pat "));
            LOG(i);
            LOG(STRING(": "));
            LOG(patValues.pats[i].pat, NEWLINE);
        }
    }
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageType pageType,
                      U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virt) >> 48L) == 0x0000 || ((virt) >> 48L) == 0xFFFF);

    U64 pageSize = pageTypeToPageSize[pageType];
    U64 depth = pageTypeToDepth[pageType];

    U64 virtualEnd = virtual + pageSize * memory.numberOfPages;
    for (U64 physical = memory.pageStart; virtual < virtualEnd;
         virtual += pageSize, physical += pageSize) {
        VirtualPageTable *currentTable = level4PageTable;

        U64 indexShift = LEVEL_4_SHIFT;
        for (U8 i = 0; i < depth; i++, indexShift -= PAGE_TABLE_SHIFT) {
            currentTable =
                (VirtualPageTable *)currentTable->pages[ALIGN_DOWN_EXP(
                    (virtual >> indexShift), PAGE_FRAME_SHIFT)];

            ASSERT((i == depth - 1) && !currentTable);

            if (!currentTable) {
                U64 value = PAGE_PRESENT | PAGE_WRITABLE | additionalFlags;
                if (i == depth - 1) {
                    value |= physical;
                    if (pageType == HUGE_PAGE || pageType == LARGE_PAGE) {
                        value |= PAGE_EXTENDED_SIZE;
                    }
                } else {
                    value |= getZeroBasePage();
                }
                currentTable = (VirtualPageTable *)value;
            }
        }
    }
}
