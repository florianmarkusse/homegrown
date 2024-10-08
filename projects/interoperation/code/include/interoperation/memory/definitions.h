#ifndef MEMORY_DEFINITIONS_H
#define MEMORY_DEFINITIONS_H

static constexpr auto RED_ZONE_SIZE = (1 << 7);

static constexpr auto LOWER_HALF_END = 0x0000FFFFFFFFFFFF;
static constexpr auto HIGHER_HALF_START = 0xffff800000000000;
static constexpr auto KERNEL_SPACE_START = 0xfffffffff8000000;
static constexpr auto KERNEL_SPACE_END =
    0xfffffffffffff000; // should be fff at the end but otherwise the memory
                        // allocation map starts to complain

static constexpr auto KERNEL_CODE_START = KERNEL_SPACE_START;

// PAGE TABLE DEFINITIONS FOR X86_64 !!!
// TODO: These macros are quite confusing, should rewrite them into more
// sensible constructs when working on virtual memory.
static constexpr auto PAGE_TABLE_SHIFT = 9ULL;
static constexpr auto PAGE_TABLE_ENTRIES = (1ULL << PAGE_TABLE_SHIFT);
static constexpr auto PAGE_TABLE_MASK = (PAGE_TABLE_ENTRIES - 1);

static constexpr auto PAGE_FRAME_SHIFT = 12ULL;
static constexpr auto PAGE_FRAME_SIZE = (1ULL << PAGE_FRAME_SHIFT);
static constexpr auto LARGE_PAGE_SIZE = (PAGE_FRAME_SIZE << PAGE_TABLE_SHIFT);
static constexpr auto HUGE_PAGE_SIZE = (LARGE_PAGE_SIZE << PAGE_TABLE_SHIFT);
static constexpr auto JUMBO_PAGE_SIZE =
    (HUGE_PAGE_SIZE
     << PAGE_TABLE_SHIFT); // Does not exist but comes in handy. 512GiB
static constexpr auto WUMBO_PAGE_SIZE =
    (JUMBO_PAGE_SIZE
     << PAGE_TABLE_SHIFT); // Does not exist but comes in handy. 256TiB
static constexpr auto PAGE_MASK = (PAGE_FRAME_SIZE - 1);

static constexpr auto LEVEL_4_SHIFT = 39U;
static constexpr auto LEVEL_3_SHIFT = 30U;
static constexpr auto LEVEL_2_SHIFT = 21U;
static constexpr auto LEVEL_1_SHIFT = 12U;

static constexpr auto PAGE_PRESENT =
    (1ULL << 0); // The page is currently in memory
static constexpr auto PAGE_WRITABLE =
    (1ULL << 1); // It’s allowed to write to this page
static constexpr auto PAGE_USER_ACCESSIBLE =
    (1ULL << 2); // If not set, only kernel mode code can access this page
static constexpr auto PAGE_WRITE_THROUGH =
    (1ULL << 3); // Writes go directly to memory
static constexpr auto PAGE_DISABLE_CACHE =
    (1ULL << 4); // No cache is used for this page
static constexpr auto PAGE_ACCESSED =
    (1ULL << 5); // The CPU sets this bit when this page is used
static constexpr auto PAGE_DIRTY =
    (1ULL << 6); // The CPU sets this bit when a write to this page occurs
static constexpr auto PAGE_EXTENDED_SIZE =
    (1ULL << 7); // Must be 0 in level 4 and 1. Created Huge/large page in level
                 // 3/2
static constexpr auto PAGE_GLOBAL =
    (1ULL << 8); // Page isn’t f6lushed from caches on address space switch (PGE
                 // bit of CR4 register must be set)
static constexpr auto PAGE_AVAILABLE_9 =
    (1ULL << 9); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_10 =
    (1ULL << 10); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_11 =
    (1ULL << 11); // Can be used freely by the OS
static constexpr auto PAGE_PHYSICAL_ADDR_MASK =
    0x7FFFFFFFFFFFFULL; // Mask for the 52-bit physical address
static constexpr auto PAGE_AVAILABLE_52 =
    (1ULL << 52); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_53 =
    (1ULL << 53); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_54 =
    (1ULL << 54); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_55 =
    (1ULL << 55); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_56 =
    (1ULL << 56); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_57 =
    (1ULL << 57); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_58 =
    (1ULL << 58); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_59 =
    (1ULL << 59); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_60 =
    (1ULL << 60); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_61 =
    (1ULL << 61); // Can be used freely by the OS
static constexpr auto PAGE_AVAILABLE_62 =
    (1ULL << 62); // Can be used freely by the OS
static constexpr auto PAGE_NO_EXECUTE =
    (1ULL << 63); // Forbid executing code on this page (the NXE bit in the EFER
                  // register must be set)

static constexpr auto KERNEL_PARAMS_SIZE = PAGE_FRAME_SIZE;
static constexpr auto KERNEL_PARAMS_START =
    (KERNEL_SPACE_END - KERNEL_PARAMS_SIZE);

static constexpr auto STACK_SIZE = (1ULL << 14);
static constexpr auto BOTTOM_STACK = (KERNEL_PARAMS_START - STACK_SIZE);
// #define KERNEL_STACK_START 0xfffffffff6000000

typedef enum {
    PAT_0 = 0,
    PAT_1 = PAGE_WRITE_THROUGH,
    PAT_2 = PAGE_DISABLE_CACHE,
    PAT_3 = PAGE_WRITE_THROUGH | PAGE_DISABLE_CACHE,
    // NOTE: The below PATs can NOT be used by extended sizes as the CPU
    // will think u set a large/huge page and select pat x - 4
    PAT_4 = PAGE_EXTENDED_SIZE,
    PAT_5 = PAGE_EXTENDED_SIZE | PAGE_WRITE_THROUGH,
    PAT_6 = PAGE_EXTENDED_SIZE | PAGE_DISABLE_CACHE,
    PAT_7 = PAGE_EXTENDED_SIZE | PAGE_DISABLE_CACHE | PAGE_WRITE_THROUGH
} PATMapping;

#define BYTES_TO_PAGE_FRAMES(a)                                                \
    (((a) >> PAGE_FRAME_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

#endif
