#ifndef CPU_IDT_H
#define CPU_IDT_H

#include "interoperation/types.h"
#include "text/string.h"
typedef struct {
    U16 limit;
    U64 base;
} __attribute__((packed)) idt_ptr;

typedef struct {
    U16 offset_1; // offset bits 0..15
    U16 selector; // a code segment selector in GDT or LDT
    U8 ist; // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    U8 type_attributes; // gate type, dpl, and p fields
    U16 offset_2;       // offset bits 16..31
    U32 offset_3;       // offset bits 32..63
    U32 zero;           // reserved
} __attribute__((packed)) InterruptDescriptor;

void initIDT();

typedef enum : U64 {
    FAULT_DIVIDE_ERROR = 0,
    FAULT_DEBUG = 1,
    FAULT_NMI = 2,
    FAULT_BREAKPOINT = 3,
    FAULT_OVERFLOW = 4,
    FAULT_BOUND_RANGE_EXCEED = 5,
    FAULT_INVALID_OPCODE = 6,
    FAULT_DEVICE_NOT_AVAILABLE = 7,
    FAULT_DOUBLE_FAULT = 8,
    FAULT_9_RESERVED = 9,
    FAULT_INVALID_TSS = 10,
    FAULT_SEGMENT_NOT_PRESENT = 11,
    FAULT_STACK_FAULT = 12,
    FAULT_GENERAL_PROTECTION = 13,
    FAULT_PAGE_FAULT = 14,
    FAULT_15_RESERVED = 15,
    FAULT_FPU_ERROR = 16,
    FAULT_ALIGNMENT_CHECK = 17,
    FAULT_MACHINE_CHECK = 18,
    FAULT_SIMD_FLOATING_POINT = 19,
    FAULT_VIRTUALIZATION = 20,
    FAULT_CONTROL_PROTECTION = 21,
    FAULT_22_RESERVED = 22,
    FAULT_23_RESERVED = 23,
    FAULT_24_RESERVED = 24,
    FAULT_25_RESERVED = 25,
    FAULT_26_RESERVED = 26,
    FAULT_27_RESERVED = 27,
    FAULT_28_RESERVED = 28,
    FAULT_29_RESERVED = 29,
    FAULT_30_RESERVED = 30,
    FAULT_31_RESERVED = 31,
    // User defined faults start here
    FAULT_USER = 32,
    FAULT_SYSCALL = 33,
    FAULT_NO_MORE_PHYSICAL_MEMORY = 34,
    FAULT_OVERLAPPING_VIRTUAL_SEGMENTS = 35,

    // Keep this to know how many we have defined
    FAULT_NUMS
} Fault;

static string faultToString[FAULT_NUMS] = {
    STRING("Divide Error"),
    STRING("Debug"),
    STRING("Non-Maskable Interrupt"),
    STRING("Breakpoint"),
    STRING("Overflow"),
    STRING("Bound Range Exceeded"),
    STRING("Invalid Opcode"),
    STRING("Device Not Available"),
    STRING("Double Fault"),
    STRING("Reserved fault"),
    STRING("Invalid TSS"),
    STRING("Segment Not Present"),
    STRING("Stack-Segment Fault"),
    STRING("General Protection"),
    STRING("Page Fault"),
    STRING("Reserved fault"),
    STRING("x87 FPU Floating-Point Error"),
    STRING("Alignment Check"),
    STRING("Machine Check"),
    STRING("SIMD Floating-Point Exception"),
    STRING("Virtualization Exception"),
    STRING("Control Protection Exception"),
    STRING("Reserved fault _ 22"),
    STRING("Reserved fault _ 23"),
    STRING("Reserved fault _ 24"),
    STRING("Reserved fault _ 25"),
    STRING("Reserved fault _ 26"),
    STRING("Reserved fault _ 27"),
    STRING("Reserved fault _ 28"),
    STRING("Reserved fault _ 29"),
    STRING("Reserved fault _ 30"),
    STRING("Reserved fault _ 31"),
    STRING("User Defined"),
    STRING("System Call"),
    STRING("No more physical memory"),
};

__attribute__((noreturn)) void triggerFault(Fault fault);

#ifdef UNIT_TEST_BUILD
void initIDTTest(void *long_jmp[5]);
bool *getTriggeredFaults();
void resetTriggeredFaults();

bool compareInterrupts(bool *expectedFaults);
void appendInterrupt(Fault fault);
void appendExpectedInterrupt(Fault fault);
void appendInterrupts(bool *expectedFaults);

#endif

#endif
