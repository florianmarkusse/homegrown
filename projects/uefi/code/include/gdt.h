#ifndef GDT_H
#define GDT_H

#include "efi/c-efi-base.h"
typedef struct {
    CEfiU16 limit15_0;
    CEfiU16 base15_0;
    CEfiU8 base23_16;
    CEfiU8 type;
    CEfiU8 limit19_16_and_flags;
    CEfiU8 base31_24;
} __attribute__((packed)) gdt_entry;

typedef struct {
    CEfiU32 reserved0;
    CEfiU64 rsp0;
    CEfiU64 rsp1;
    CEfiU64 rsp2;
    CEfiU64 reserved1;
    CEfiU64 ist1;
    CEfiU64 ist2;
    CEfiU64 ist3;
    CEfiU64 ist4;
    CEfiU64 ist5;
    CEfiU64 ist6;
    CEfiU64 ist7;
    CEfiU64 reserved2;
    CEfiU16 reserved3;
    CEfiU16 iopb_offset;
} __attribute__((packed)) tss;

typedef struct {
    gdt_entry null;
    gdt_entry kernel_code;
    gdt_entry kernel_data;
} __attribute__((aligned(4096))) gdtable;

// No other descriptors necessary afaik, no hardware switching nor privilege
// switching. Just straight balling.
const static gdtable gdt_table = {
    {0, 0, 0, 0x00, 0x00, 0}, /* 0x00 null  */
    {0, 0, 0, 0x9a, 0xa0, 0}, /* 0x08 kernel code (kernel base selector) */
    {0, 0, 0, 0x92, 0xa0, 0}, /* 0x10 kernel data */
};

extern /* defined in assembly */
    void
    load_gdt(CEfiU16 limit, CEfiU64 base);

void setup_gdt();

#endif
