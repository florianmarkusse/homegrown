#ifndef IDT_H
#define IDT_H

#include "util/types.h"
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr;

typedef struct {
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t
        ist; // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;       // offset bits 16..31
    uint32_t offset_3;       // offset bits 32..63
    uint32_t zero;           // reserved
} __attribute__((packed)) InterruptDescriptor;

void setupIDT();

#endif
