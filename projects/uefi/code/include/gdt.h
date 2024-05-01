#ifndef GDT_H
#define GDT_H

#include "efi/c-efi-base.h"

typedef struct {
    union {
        struct {
            CEfiU64 limit_15_0 : 16;
            CEfiU64 base_15_0 : 16;
            CEfiU64 base_23_16 : 8;
            CEfiU64 type : 4;
            CEfiU64 s_flag : 1;
            CEfiU64 dpl : 2;
            CEfiU64 p_flag : 1;
            CEfiU64 limit_19_16 : 4;
            CEfiU64 avl : 1;
            CEfiU64 l_flag : 1;
            CEfiU64 db_flag : 1;
            CEfiU64 g_flag : 1;
            CEfiU64 base_31_24 : 8;
        };
        CEfiU64 value;
    };
} __attribute__((packed)) Segment_Descriptor;

typedef struct {
    union {
        struct {
            CEfiU64 limit_15_0 : 16;
            CEfiU64 base_15_0 : 16;
            CEfiU64 base_23_16 : 8;
            CEfiU64 type : 4;
            CEfiU64 zero_1 : 1;
            CEfiU64 dpl : 2;
            CEfiU64 p_flag : 1;
            CEfiU64 limit_19_16 : 4;
            CEfiU64 avl : 1;
            CEfiU64 zero_2 : 1;
            CEfiU64 zero_3 : 1;
            CEfiU64 g_flag : 1;
            CEfiU64 base_31_24 : 8;
        };
        CEfiU64 value;
    };

    CEfiU64 base_63_32 : 32;
    CEfiU64 reserved_1 : 8;
    CEfiU64 zero_4 : 5;
    CEfiU64 reserved_2 : 19;
} __attribute__((packed)) Tss_Descriptor;

// Task state segment - 64 bit
typedef struct {
    CEfiU32 reserved_0;
    CEfiU32 rsp0_low;
    CEfiU32 rsp0_high;
    CEfiU32 rsp1_low;
    CEfiU32 rsp1_high;
    CEfiU32 rsp2_low;
    CEfiU32 rsp2_high;
    CEfiU32 reserved_1;
    CEfiU32 reserved_2;
    CEfiU32 ist1_low;
    CEfiU32 ist1_high;
    CEfiU32 ist2_low;
    CEfiU32 ist2_high;
    CEfiU32 ist3_low;
    CEfiU32 ist3_high;
    CEfiU32 ist4_low;
    CEfiU32 ist4_high;
    CEfiU32 ist5_low;
    CEfiU32 ist5_high;
    CEfiU32 ist6_low;
    CEfiU32 ist6_high;
    CEfiU32 ist7_low;
    CEfiU32 ist7_high;
    CEfiU32 reserved_3;
    CEfiU32 reserved_4;
    CEfiU16 reserved_5;
    CEfiU16 io_map_base_address;
} __attribute__((packed)) Task_State_Segment;

typedef struct {
    Segment_Descriptor null;
    Segment_Descriptor kernel_code;
    Segment_Descriptor kernel_data;
    Tss_Descriptor tssDescriptor;
} __attribute__((aligned(4096))) gdtable;

const static Task_State_Segment tss = {0};
const static CEfiU64 tss_address = (CEfiU64)&tss;

// No other descriptors necessary afaik, no hardware switching nor privilege
// switching. Just straight balling.
// TSS descriptor is filled out later because it is not a static constant.
static gdtable gdt_table = {
    .null = {0},
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
};

typedef struct {
    CEfiU16 limit;
    CEfiU64 base;
} __attribute__((packed)) Descriptor_Table_Register;

const static Descriptor_Table_Register gdtr = {.limit = sizeof(gdt_table) - 1,
                                               .base = (CEfiU64)&gdt_table};

void setupNewGDT();

#endif
