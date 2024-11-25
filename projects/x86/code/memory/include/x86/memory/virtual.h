#ifndef X86_MEMORY_VIRTUAL_H
#define X86_MEMORY_VIRTUAL_H

#include "interoperation/types.h"

constexpr struct {
    U64 SHIFT;
    U64 ENTRIES;
} PageTableFormat = {.SHIFT = 9, .ENTRIES = (1ULL << PageTableFormat.SHIFT)};

U64 arraysOfThings[PageTableFormat.ENTRIES];

// PAGE TABLE DEFINITIONS FOR X86_64 !!!
// TODO: These macros are quite confusing, should rewrite them into more
// sensible constructs when working on virtual memory.
static constexpr auto PAGE_FRAME_SHIFT = 12ULL;
static constexpr auto PAGE_FRAME_SIZE = (1ULL << PAGE_FRAME_SHIFT);
static constexpr auto LARGE_PAGE_SIZE =
    (PAGE_FRAME_SIZE << PageTableFormat.SHIFT);
static constexpr auto HUGE_PAGE_SIZE =
    (LARGE_PAGE_SIZE << PageTableFormat.SHIFT);
static constexpr auto JUMBO_PAGE_SIZE =
    (HUGE_PAGE_SIZE
     << PageTableFormat.SHIFT); // Does not exist but comes in handy. 512GiB
static constexpr auto WUMBO_PAGE_SIZE =
    (JUMBO_PAGE_SIZE
     << PageTableFormat.SHIFT); // Does not exist but comes in handy. 256TiB
static constexpr auto PAGE_MASK = (PAGE_FRAME_SIZE - 1);

static constexpr auto LEVEL_4_SHIFT = 39U;
static constexpr auto LEVEL_3_SHIFT = 30U;
static constexpr auto LEVEL_2_SHIFT = 21U;
static constexpr auto LEVEL_1_SHIFT = 12U;

static constexpr auto NUM_PAGE_SIZES = 3;
typedef enum : U64 {
    BASE_PAGE = PAGE_FRAME_SIZE,
    LARGE_PAGE = LARGE_PAGE_SIZE,
    HUGE_PAGE = HUGE_PAGE_SIZE,
} PageSize;

extern PageSize pageSizes[NUM_PAGE_SIZES];

constexpr struct {
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

#endif
