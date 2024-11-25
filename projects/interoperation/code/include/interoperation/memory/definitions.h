#ifndef MEMORY_DEFINITIONS_H
#define MEMORY_DEFINITIONS_H

#include "interoperation/types.h"
#include "x86/memory/virtual.h"

static constexpr auto RED_ZONE_SIZE = (1 << 7);

// TODO: Fix this once uefi code is improved!!!
static constexpr auto LOWER_HALF_END = 0x0000FFFFFFFFFFFF;
static constexpr auto HIGHER_HALF_START = 0xffff800000000000;
static constexpr auto KERNEL_SPACE_START = 0xfffffffff8000000;
static constexpr auto KERNEL_SPACE_END =
    0xfffffffffffff000; // should be fff at the end but otherwise the memory
                        // allocation map starts to complain
static constexpr auto KERNEL_CODE_START = KERNEL_SPACE_START;

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
