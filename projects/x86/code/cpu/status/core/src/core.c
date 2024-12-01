#include "x86/cpu/status/core.h"
#include "platform-abstraction/idt.h"
#include "platform-abstraction/log.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

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
    STRING("Allocation request was too large"),
};

void appendInterrupt(Fault fault) {
    LOG(STRING("Fault #: "));
    LOG(fault);
    LOG(STRING("\tMsg: "));
    LOG(stringWithMinSizeDefault(faultToString[fault], 30));
}
