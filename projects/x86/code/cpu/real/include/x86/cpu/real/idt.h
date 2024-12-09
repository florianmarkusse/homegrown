#ifndef X86_CPU_REAL_IDT_H
#define X86_CPU_REAL_IDT_H

#include "shared/types/types.h"
#include "x86/cpu/fault.h"

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
__attribute__((noreturn)) void triggerFault(Fault fault);

#endif
