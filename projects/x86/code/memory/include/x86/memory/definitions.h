#ifndef X86_MEMORY_DEFINITIONS_H
#define X86_MEMORY_DEFINITIONS_H

#include "shared/enum.h"
#include "shared/types/types.h"

static constexpr struct {
    U64 ENTRIES;
} PageTableFormat = {.ENTRIES = (1ULL << 9ULL)};

static constexpr auto PAGE_FRAME_SHIFT = 12ULL;
static constexpr auto PAGE_FRAME_SIZE = (1ULL << PAGE_FRAME_SHIFT);
static constexpr auto LARGE_PAGE_SIZE =
    (PAGE_FRAME_SIZE * PageTableFormat.ENTRIES);
static constexpr auto HUGE_PAGE_SIZE =
    (LARGE_PAGE_SIZE * PageTableFormat.ENTRIES);
static constexpr auto JUMBO_PAGE_SIZE =
    (HUGE_PAGE_SIZE *
     PageTableFormat.ENTRIES); // Does not exist but comes in handy. 512GiB
static constexpr auto WUMBO_PAGE_SIZE =
    (JUMBO_PAGE_SIZE *
     PageTableFormat.ENTRIES); // Does not exist but comes in handy. 256TiB
static constexpr auto PAGE_MASK = (PAGE_FRAME_SIZE - 1);

#define MEMORY_PAGE_SIZES_ENUM(VARIANT)                                        \
    VARIANT(BASE_PAGE, PAGE_FRAME_SIZE)                                        \
    VARIANT(LARGE_PAGE, LARGE_PAGE_SIZE)                                       \
    VARIANT(HUGE_PAGE, HUGE_PAGE_SIZE)

typedef enum : U64 { MEMORY_PAGE_SIZES_ENUM(ENUM_VALUES_VARIANT) } PageSize;
static constexpr auto MEMORY_PAGE_SIZES_COUNT =
    (0 MEMORY_PAGE_SIZES_ENUM(PLUS_ONE));

// NOTE: Goes from smallest to largest!!!
extern PageSize pageSizes[MEMORY_PAGE_SIZES_COUNT];

static constexpr U64 AVAILABLE_PAGE_SIZES_MASK =
    (PAGE_FRAME_SIZE | LARGE_PAGE_SIZE | HUGE_PAGE_SIZE);

typedef struct {
    U8 data[PAGE_FRAME_SIZE];
} PhysicalBasePage;

typedef struct {
    union {
        U8 data[LARGE_PAGE_SIZE];
        PhysicalBasePage basePages[PageTableFormat.ENTRIES];
    };
} PhysicalLargePage;

typedef struct {
    union {
        U8 data[HUGE_PAGE_SIZE];
        PhysicalBasePage
            basePages[PageTableFormat.ENTRIES * PageTableFormat.ENTRIES];
        PhysicalLargePage largePages[PageTableFormat.ENTRIES];
    };
} PhysicalHugePage;

typedef struct {
    union {
        U64 value;
        struct {
            U64 present : 1;
            U64 writable : 1;
            U64 userAccessible : 1;
            U64 writeThrough : 1;
            U64 disableCache : 1;
            U64 accessed : 1;
            U64 dirty : 1;
            U64 extendedSize : 1;
            U64 global : 1;
            U64 available_9 : 1;
            U64 available_10 : 1;
            U64 available_11 : 1;
            U64 level_1 : 9;
            U64 level_2 : 9;
            U64 level_3 : 9;
            U64 level_4 : 9;
            U64 available_52 : 1;
            U64 available_53 : 1;
            U64 available_54 : 1;
            U64 available_55 : 1;
            U64 available_56 : 1;
            U64 available_57 : 1;
            U64 available_58 : 1;
            U64 available_59 : 1;
            U64 available_60 : 1;
            U64 available_61 : 1;
            U64 available_62 : 1;
            U64 noExecute : 1;
        };
    };
} VirtualEntry;

static constexpr struct {
    U64 PAGE_PRESENT;         // The page is currently in memory
    U64 PAGE_WRITABLE;        // It’s allowed to write to this page
    U64 PAGE_USER_ACCESSIBLE; // If not set, only kernel mode code can access
                              // this page
    U64 PAGE_WRITE_THROUGH;   // Writes go directly to memory
    U64 PAGE_DISABLE_CACHE;   // No cache is used for this page
    U64 PAGE_ACCESSED;        // The CPU sets this bit when this page is used
    U64 PAGE_DIRTY; // The CPU sets this bit when a write to this page occurs
    U64 PAGE_EXTENDED_SIZE; // Must be 0 in level 4 and 1. Created
                            // Huge/large page in level 3/2
    U64 PAGE_GLOBAL; // Page isn’t f6lushed from caches on address space switch
                     // (PGE bit of CR4 register must be set)
    U64 PAGE_AVAILABLE_9;  // Can be used freely by the OS
    U64 PAGE_AVAILABLE_10; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_11; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_52; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_53; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_54; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_55; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_56; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_57; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_58; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_59; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_60; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_61; // Can be used freely by the OS
    U64 PAGE_AVAILABLE_62; // Can be used freely by the OS
    U64 PAGE_NO_EXECUTE;   // Forbid executing code on this page (the NXE bit in
                           // the EFER register must be set)
    U64 FRAME_OR_NEXT_PAGE_TABLE; // To get the physical address or the next
                                  // page table address of a virtual address.
} VirtualPageMasks = {.PAGE_PRESENT = (1ULL << 0),
                      .PAGE_WRITABLE = (1ULL << 1),
                      .PAGE_USER_ACCESSIBLE = (1ULL << 2),
                      .PAGE_WRITE_THROUGH = (1ULL << 3),
                      .PAGE_DISABLE_CACHE = (1ULL << 4),
                      .PAGE_ACCESSED = (1ULL << 5),
                      .PAGE_DIRTY = (1ULL << 6),
                      .PAGE_EXTENDED_SIZE = (1ULL << 7),
                      .PAGE_GLOBAL = (1ULL << 8),
                      .PAGE_AVAILABLE_9 = (1ULL << 9),
                      .PAGE_AVAILABLE_10 = (1ULL << 10),
                      .PAGE_AVAILABLE_11 = (1ULL << 11),
                      .PAGE_AVAILABLE_52 = (1ULL << 52),
                      .PAGE_AVAILABLE_53 = (1ULL << 53),
                      .PAGE_AVAILABLE_54 = (1ULL << 54),
                      .PAGE_AVAILABLE_55 = (1ULL << 55),
                      .PAGE_AVAILABLE_56 = (1ULL << 56),
                      .PAGE_AVAILABLE_57 = (1ULL << 57),
                      .PAGE_AVAILABLE_58 = (1ULL << 58),
                      .PAGE_AVAILABLE_59 = (1ULL << 59),
                      .PAGE_AVAILABLE_60 = (1ULL << 60),
                      .PAGE_AVAILABLE_61 = (1ULL << 61),
                      .PAGE_AVAILABLE_62 = (1ULL << 62),
                      .PAGE_NO_EXECUTE = (1ULL << 63),
                      .FRAME_OR_NEXT_PAGE_TABLE = 0x000FFFFFFFFF000};

typedef struct {
    U64 start;
    U64 end;
} VirtualRegion;

typedef struct {
    U64 pages[PageTableFormat.ENTRIES];
} VirtualPageTable;

typedef struct {
    VirtualEntry entry;
    PageSize pageSize;
} MappedPage;

#endif
