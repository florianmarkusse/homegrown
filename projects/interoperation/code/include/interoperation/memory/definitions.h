#ifndef INTEROPERATION_MEMORY_DEFINITIONS_H
#define INTEROPERATION_MEMORY_DEFINITIONS_H

static constexpr auto LOWER_HALF_END = 0x0000FFFFFFFFFFFF;
static constexpr auto HIGHER_HALF_START = 0xffff800000000000;
static constexpr auto KERNEL_SPACE_START = 0xfffffffff8000000;
static constexpr auto KERNEL_SPACE_END =
    0xfffffffffffff000; // should be fff at the end but otherwise the memory
                        // allocation map starts to complain
static constexpr auto KERNEL_CODE_START = KERNEL_SPACE_START;

static constexpr auto KERNEL_PARAMS_SIZE = (1 << 12);
static constexpr auto KERNEL_PARAMS_START =
    (KERNEL_SPACE_END - KERNEL_PARAMS_SIZE);

static constexpr auto STACK_SIZE = (1ULL << 14);
static constexpr auto BOTTOM_STACK = (KERNEL_PARAMS_START - STACK_SIZE);

#endif
