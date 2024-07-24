#include "gdt.h"
#include "efi/c-efi-base.h"        // for PhysicalAddress
#include "memory/boot-functions.h" // for allocAndZero
#include "types.h"                 // for U64, U32, U16

typedef struct {
    union {
        struct {
            U64 limit_15_0 : 16;
            U64 base_15_0 : 16;
            U64 base_23_16 : 8;
            U64 type : 4;
            U64 s_flag : 1;
            U64 dpl : 2;
            U64 p_flag : 1;
            U64 limit_19_16 : 4;
            U64 avl : 1;
            U64 l_flag : 1;
            U64 db_flag : 1;
            U64 g_flag : 1;
            U64 base_31_24 : 8;
        };
        U64 value;
    };
} __attribute__((packed)) Segment_Descriptor;

typedef struct {
    union {
        struct {
            U64 limit_15_0 : 16;
            U64 base_15_0 : 16;
            U64 base_23_16 : 8;
            U64 type : 4;
            U64 zero_1 : 1;
            U64 dpl : 2;
            U64 p_flag : 1;
            U64 limit_19_16 : 4;
            U64 avl : 1;
            U64 zero_2 : 1;
            U64 zero_3 : 1;
            U64 g_flag : 1;
            U64 base_31_24 : 8;
        };
        U64 value;
    };

    U64 base_63_32 : 32;
    U64 reserved_1 : 8;
    U64 zero_4 : 5;
    U64 reserved_2 : 19;
} __attribute__((packed)) Tss_Descriptor;

// Task state segment - 64 bit
typedef struct {
    U32 reserved_0;
    U32 rsp0_low;
    U32 rsp0_high;
    U32 rsp1_low;
    U32 rsp1_high;
    U32 rsp2_low;
    U32 rsp2_high;
    U32 reserved_1;
    U32 reserved_2;
    U32 ist1_low;
    U32 ist1_high;
    U32 ist2_low;
    U32 ist2_high;
    U32 ist3_low;
    U32 ist3_high;
    U32 ist4_low;
    U32 ist4_high;
    U32 ist5_low;
    U32 ist5_high;
    U32 ist6_low;
    U32 ist6_high;
    U32 ist7_low;
    U32 ist7_high;
    U32 reserved_3;
    U32 reserved_4;
    U16 reserved_5;
    U16 io_map_base_address;
} __attribute__((packed)) Task_State_Segment;

typedef struct {
    Segment_Descriptor null;
    Segment_Descriptor kernel_code;
    Segment_Descriptor kernel_data;
    Tss_Descriptor tssDescriptor;
} __attribute__((packed)) gdtable;

typedef struct {
    U16 limit;
    U64 base;
} __attribute__((packed)) Descriptor_Table_Register;

Descriptor_Table_Register *new_gdtr;

void prepNewGDT() {
    PhysicalAddress zeroPage = allocAndZero(1);
    Task_State_Segment *new_tss = (Task_State_Segment *)zeroPage;
    new_tss->io_map_base_address = sizeof(Task_State_Segment);
    U64 new_tss_address = zeroPage;

    zeroPage = allocAndZero(1);
    gdtable *new_gdt = (gdtable *)zeroPage;
    *new_gdt = (gdtable){.null = {0},
                         .kernel_code = {.limit_15_0 = 0xFFFF,
                                         .base_15_0 = 0,
                                         .base_23_16 = 0,
                                         .type = 0xA,
                                         .s_flag = 1,
                                         .dpl = 0,
                                         .p_flag = 1,
                                         .limit_19_16 = 0xF,
                                         .avl = 0,
                                         .l_flag = 1,
                                         .db_flag = 0,
                                         .g_flag = 1,
                                         .base_31_24 = 0},
                         .kernel_data = {.limit_15_0 = 0xFFFF,
                                         .base_15_0 = 0,
                                         .base_23_16 = 0,
                                         .type = 0x2,
                                         .s_flag = 1,
                                         .dpl = 0,
                                         .p_flag = 1,
                                         .limit_19_16 = 0xF,
                                         .avl = 0,
                                         .l_flag = 0,
                                         .db_flag = 1,
                                         .g_flag = 1,
                                         .base_31_24 = 0},
                         .tssDescriptor = {
                             .limit_15_0 = sizeof(Task_State_Segment) - 1,
                             .base_15_0 = new_tss_address & 0xFFFF,
                             .base_23_16 = (new_tss_address >> 16) & 0xFF,
                             .type = 9, // 0b1001 = 64 bit TSS (available)
                             .p_flag = 1,
                             .base_31_24 = (new_tss_address >> 24) & 0xFF,
                             .base_63_32 = (new_tss_address >> 32) & 0xFFFFFFFF,

                         }};

    zeroPage = allocAndZero(1);
    new_gdtr = (Descriptor_Table_Register *)zeroPage;
    *new_gdtr = (Descriptor_Table_Register){.limit = sizeof(gdtable) - 1,
                                            .base = (U64)new_gdt};
}
void enableNewGDT() {
    __asm__ __volatile__(
        "lgdt %0;" // Load new Global Descriptor Table

        "movw $0x18, %%ax;" // 0x18 = tss segment offset in gdt
        "ltr %%ax;"         // Load task register, with tss offset

        "movq $0x08, %%rax;"
        "pushq %%rax;"           // Push kernel code segment
        "leaq 1f(%%rip), %%rax;" // Use relative offset for label
        "pushq %%rax;"           // Push return address
        "lretq;"                 // Far return, pop return address into IP,
                                 //   and pop code segment into CS
        "1:"
        "movw $0x10, %%ax;" // 0x10 = kernel data segment
        "movw %%ax, %%ds;"  // Load data segment registers
        "movw %%ax, %%es;"
        "movw %%ax, %%fs;"
        "movw %%ax, %%gs;"
        "movw %%ax, %%ss;"

        :
        : "m"(*new_gdtr)
        : "rax", "memory");
}
