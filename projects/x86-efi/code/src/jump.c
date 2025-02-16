#include "efi-to-kernel/memory/definitions.h"
#include "efi/globals.h"
#include "platform-abstraction/efi.h"
#include "x86-efi/gdt.h"
#include "x86-virtual.h"
#include "x86/gdt.h"

void enableNewMemoryMapping() {
    asm volatile("mov %%rax, %%cr3"
                         :
                         : "a"(level4PageTable)
                         : "memory");
}

void toKernel(U64 stackPointer) {
    asm volatile("movq %0, %%rsp;"
                         "movq %%rsp, %%rbp;"
                         "cld;"
                         "pushq %1;"
                         "retq"
                         :
                         : "r"(stackPointer), "r"(KERNEL_CODE_START)
                         : "memory");
}

void jumpIntoKernel(U64 stackPointer) {
    enableNewGDT(gdtDescriptor);
    enableNewMemoryMapping();
    toKernel(stackPointer);

    __builtin_unreachable();
}
