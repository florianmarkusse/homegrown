#ifndef MEMORY_DEFINITIONS_H
#define MEMORY_DEFINITIONS_H

#include "interoperation/types.h"
static constexpr auto RED_ZONE_SIZE = (1 << 7);

// TODO: Fix this once uefi code is improved!!!
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

static constexpr auto KERNEL_PARAMS_SIZE = PAGE_FRAME_SIZE;
static constexpr auto KERNEL_PARAMS_START =
    (KERNEL_SPACE_END - KERNEL_PARAMS_SIZE);

static constexpr auto STACK_SIZE = (1ULL << 14);
static constexpr auto BOTTOM_STACK = (KERNEL_PARAMS_START - STACK_SIZE);
// #define KERNEL_STACK_START 0xfffffffff6000000

// TODO: Fix this once UEFI code is improved!!
#define BYTES_TO_PAGE_FRAMES(a)                                                \
    (((a) >> PAGE_FRAME_SHIFT) + ((a) & PAGE_MASK ? 1 : 0))

#endif
