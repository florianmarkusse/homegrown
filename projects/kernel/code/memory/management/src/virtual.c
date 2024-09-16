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

static U8 pageSizeToDepth(PageSize pageSize) {
    switch (pageSize) {
    case BASE_PAGE: {
        return 4;
    }
    case LARGE_PAGE: {
        return 3;
    }
    default: {
        return 2;
    }
    }
}

typedef struct {
    U64 start;
    U64 end;
} VirtualRegion;

VirtualRegion higherHalfRegion = {.start = HIGHER_HALF_START,
                                  .end = KERNEL_SPACE_START};
// Start is set in the init function.
VirtualRegion lowerHalfRegion = {.start = 0, .end = LOWER_HALF_END};

typedef struct {
    U64 pages[PAGE_TABLE_ENTRIES];
} VirtualPageTable;

static VirtualPageTable *level4PageTable;

static U64 getZeroBasePage() {
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

U64 getVirtualMemory(U64 size, PageSize alignValue) {
    ASSERT(size <= JUMBO_PAGE_SIZE);
    U64 alignedUpValue = ALIGN_UP_VALUE(higherHalfRegion.start, alignValue);

    ASSERT(higherHalfRegion.start <= alignedUpValue);
    ASSERT(alignedUpValue <= higherHalfRegion.end);
    ASSERT(higherHalfRegion.end - (alignedUpValue + size) <
           higherHalfRegion.start);

    higherHalfRegion.start = alignedUpValue;
    U64 result = higherHalfRegion.start;

    higherHalfRegion.start += size;
    return result;
}

static void appendVirtualRegionStatus(VirtualRegion region) {
    LOG(STRING("Start: "));
    LOG((void *)region.start, NEWLINE);
    LOG(STRING("End: "));
    LOG((void *)region.end, NEWLINE);
}

void appendVirtualMemoryManagerStatus() {
    LOG(STRING("Available Virtual Memory\n"));
    LOG(STRING("Lower half (0x0000_000000000000):\n"));
    appendVirtualRegionStatus(lowerHalfRegion);
    LOG(STRING("Higher half(0xFFFF_000000000000):\n"));
    appendVirtualRegionStatus(higherHalfRegion);

    LOG(STRING("CR3/root page table address is: "));
    LOG((void *)level4PageTable, NEWLINE);

    PAT patValues = {.value = rdmsr(PAT_LOCATION)};
    LOG(STRING("PAT MSR set to:\n"));
    for (U8 i = 0; i < 8; i++) {
        LOG(STRING("PAT "));
        LOG(i);
        LOG(STRING(": "));
        LOG(patEncodingToString[patValues.pats[i].pat], NEWLINE);
    }
}

static void programPat() {
    PAT patValues = {.value = rdmsr(PAT_LOCATION)};

    patValues.pats[3].pat = PAT_WRITE_COMBINGING_WC;
    wrmsr(PAT_LOCATION, patValues.value);

    flushTLB();
}

void initVirtualMemoryManager(U64 level4Address, KernelMemory kernelMemory) {
    U64 currentHighestAddress = 0;
    for (U64 i = 0;
         i < kernelMemory.totalDescriptorSize / kernelMemory.descriptorSize;
         i++) {
        MemoryDescriptor *desc =
            (MemoryDescriptor *)((U8 *)kernelMemory.descriptors +
                                 (i * kernelMemory.descriptorSize));
        U64 highestAddress =
            desc->virtualStart + desc->numberOfPages * PAGE_FRAME_SIZE;

        if (highestAddress > currentHighestAddress) {
            currentHighestAddress = highestAddress;
        }
    }
    lowerHalfRegion.start = currentHighestAddress;

    level4PageTable = (VirtualPageTable *)level4Address;
    programPat();
}

void mapVirtualRegion(U64 virtual, PagedMemory memory, PageSize pageType) {
    mapVirtualRegionWithFlags(virtual, memory, pageType, 0);
}

// The caller should take care that the virtual address and physical
// address are correctly aligned. If they are not, not sure what the
// caller wanted to accomplish.
void mapVirtualRegionWithFlags(U64 virtual, PagedMemory memory,
                               PageSize pageType, U64 additionalFlags) {
    ASSERT(level4PageTable);
    ASSERT(((virtual) >> 48L) == 0x0000 || ((virtual) >> 48L) == 0xFFFF);

    U64 depth = pageSizeToDepth(pageType);

    ASSERT(!(RING_RANGE_VALUE(virtual, pageType)));
    ASSERT(!(RING_RANGE_VALUE(memory.pageStart, pageType)));

    U64 virtualEnd = virtual + pageType * memory.numberOfPages;
    for (U64 physical = memory.pageStart; virtual < virtualEnd;
         virtual += pageType, physical += pageType) {
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
