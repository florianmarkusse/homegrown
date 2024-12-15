#ifndef X86_CPU_FAULT_H
#define X86_CPU_FAULT_H

#include "shared/enum.h"
#include "shared/types/types.h"

#define CPU_FAULT_ENUM(VARIANT)                                                \
    VARIANT(FAULT_DIVIDE_ERROR, 0)                                             \
    VARIANT(FAULT_DEBUG, 1)                                                    \
    VARIANT(FAULT_NMI, 2)                                                      \
    VARIANT(FAULT_BREAKPOINT, 3)                                               \
    VARIANT(FAULT_OVERFLOW, 4)                                                 \
    VARIANT(FAULT_BOUND_RANGE_EXCEED, 5)                                       \
    VARIANT(FAULT_INVALID_OPCODE, 6)                                           \
    VARIANT(FAULT_DEVICE_NOT_AVAILABLE, 7)                                     \
    VARIANT(FAULT_DOUBLE_FAULT, 8)                                             \
    VARIANT(FAULT_9_RESERVED, 9)                                               \
    VARIANT(FAULT_INVALID_TSS, 10)                                             \
    VARIANT(FAULT_SEGMENT_NOT_PRESENT, 11)                                     \
    VARIANT(FAULT_STACK_FAULT, 12)                                             \
    VARIANT(FAULT_GENERAL_PROTECTION, 13)                                      \
    VARIANT(FAULT_PAGE_FAULT, 14)                                              \
    VARIANT(FAULT_15_RESERVED, 15)                                             \
    VARIANT(FAULT_FPU_ERROR, 16)                                               \
    VARIANT(FAULT_ALIGNMENT_CHECK, 17)                                         \
    VARIANT(FAULT_MACHINE_CHECK, 18)                                           \
    VARIANT(FAULT_SIMD_FLOATING_POINT, 19)                                     \
    VARIANT(FAULT_VIRTUALIZATION, 20)                                          \
    VARIANT(FAULT_CONTROL_PROTECTION, 21)                                      \
    VARIANT(FAULT_22_RESERVED, 22)                                             \
    VARIANT(FAULT_23_RESERVED, 23)                                             \
    VARIANT(FAULT_24_RESERVED, 24)                                             \
    VARIANT(FAULT_25_RESERVED, 25)                                             \
    VARIANT(FAULT_26_RESERVED, 26)                                             \
    VARIANT(FAULT_27_RESERVED, 27)                                             \
    VARIANT(FAULT_28_RESERVED, 28)                                             \
    VARIANT(FAULT_29_RESERVED, 29)                                             \
    VARIANT(FAULT_30_RESERVED, 30)                                             \
    VARIANT(FAULT_31_RESERVED, 31)                                             \
    /* User defined faults start here */                                       \
    VARIANT(FAULT_USER, 32)                                                    \
    VARIANT(FAULT_SYSCALL, 33)                                                 \
    VARIANT(FAULT_NO_MORE_PHYSICAL_MEMORY, 34)                                 \
    VARIANT(FAULT_TOO_LARGE_ALLOCATION, 35)

typedef enum : U64 { CPU_FAULT_ENUM(ENUM_VALUES_VARIANT) } Fault;
static constexpr auto CPU_FAULT_COUNT = (0 CPU_FAULT_ENUM(PLUS_ONE));

#endif
