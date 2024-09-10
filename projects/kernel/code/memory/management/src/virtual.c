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
    U64 pages[PAGE_TABLE_ENTRIES];
} VirtualPageTable;

static VirtualPageTable *level4PageTable;

U64 getZeroBasePage() {
    PagedMemory memoryForAddresses[1];
    PagedMemory_a memory =
        allocPhysicalPages((PagedMemory_a){.buf = memoryForAddresses,
                                           .len = COUNTOF(memoryForAddresses)},
                           BASE_PAGE);
    memset((void *)memory.buf[0].pageStart, 0, PAGE_FRAME_SIZE);
    return memory.buf[0].pageStart;
}

#define PAT_LOCATION 0x277

typedef enum {
    PAT_UNCACHABLE_UC = 0x0,
    PAT_WRITE_COMBINGING_WC = 0x1,
    PAT_RESERVED_2 = 0x2,
    PAT_RESERVED_3 = 0x3,
    PAT_WRITE_THROUGH_WT = 0x4,
    PAT_WRITE_PROTECTED_WP = 0x5,
    PAT_WRITE_BACK_WB = 0x6,
    PAT_UNCACHED_UC_ = 0x7,
    PAT_NUMS
} PATEncoding;

static string patEncodingToString[PAT_NUMS] = {
    STRING("Uncachable (UC)"),        STRING("Write Combining (WC)"),
    STRING("Reserved 1, don't use!"), STRING("Reserved 2, don't use!"),
    STRING("Write Through (WT)"),     STRING("Write Protected (WP)"),
    STRING("Write Back (WB)"),        STRING("Uncached (UC-)"),
};

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

void printVirtualMemoryManagerStatus() {
    FLUSH_AFTER {
        LOG(STRING("CR3/root page table address is: "));
        LOG((void *)level4PageTable, NEWLINE);
    }

    PAT patValues = {.value = rdmsr(PAT_LOCATION)};
    FLUSH_AFTER {
        LOG(STRING("PAT MSR set to:\n"));
        for (U8 i = 0; i < 8; i++) {
            LOG(STRING("PAT "));
            LOG(i);
            LOG(STRING(": "));
            LOG(patEncodingToString[patValues.pats[i].pat], NEWLINE);
        }
    }
}

void programPat() {
    PAT patValues = {.value = rdmsr(PAT_LOCATION)};

    patValues.pats[3].pat = PAT_WRITE_COMBINGING_WC;
    wrmsr(PAT_LOCATION, patValues.value);

    flushTLB();
}

void initVirtualMemoryManager(U64 level4Address) {
    level4PageTable = (VirtualPageTable *)level4Address;
    programPat();
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegion(U64 virtual, PagedMemory memory, PageType pageType,
                      U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virtual) >> 48L) == 0x0000 || ((virtual) >> 48L) == 0xFFFF);

    U64 pageSize = pageTypeToPageSize[pageType];
    U64 depth = pageTypeToDepth[pageType];

    ASSERT(!(RING_RANGE_VALUE(virtual, pageSize)));
    ASSERT(!(RING_RANGE_VALUE(memory.pageStart, pageSize)));

    if (memory.numberOfPages >= PAGE_TABLE_ENTRIES / 2) {
        FLUSH_AFTER {
            LOG(STRING("It might be better to use a larger page size to "
                       "alleviate pressure on the TLB.\n"));
            LOG(STRING("Number of pages requested to map: "));
            LOG(memory.numberOfPages, NEWLINE);
            LOG(STRING("Current Page Type: "));
            LOG(pageTypeToString[pageType], NEWLINE);
        }
    }

    U64 virtualEnd = virtual + pageSize * memory.numberOfPages;
    for (U64 physical = memory.pageStart; virtual < virtualEnd;
         virtual += pageSize, physical += pageSize) {
        VirtualPageTable *currentTable = level4PageTable;

        U64 indexShift = LEVEL_4_SHIFT;
        for (U8 i = 0; i < depth; i++, indexShift -= PAGE_TABLE_SHIFT) {
            U64 *address = &(currentTable->pages[RING_RANGE_EXP(
                (virtual >> indexShift), PAGE_TABLE_SHIFT)]);

            if (i == depth - 1) {
                U64 value =
                    PAGE_PRESENT | PAGE_WRITABLE | physical | additionalFlags;
                if (pageType == HUGE_PAGE || pageType == LARGE_PAGE) {
                    value |= PAGE_EXTENDED_SIZE;
                }
                *address = value;
            } else if (!*address) {
                U64 value = PAGE_PRESENT | PAGE_WRITABLE;
                value |= getZeroBasePage();
                *address = value;
            }

            currentTable =
                (VirtualPageTable *)ALIGN_DOWN_EXP(*address, PAGE_FRAME_SHIFT);
        }
    }
}
