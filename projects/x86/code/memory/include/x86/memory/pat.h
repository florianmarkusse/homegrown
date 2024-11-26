#ifndef X86_MEMORY_PAT_H
#define X86_MEMORY_PAT_H

#include "interoperation/memory/definitions.h"
#include "shared/types/types.h"
#include "x86/memory/virtual.h"

typedef enum {
    PAT_UNCACHABLE_UC = 0x0,
    PAT_WRITE_COMBINGING_WC = 0x1,
    PAT_RESERVED_2 = 0x2,
    PAT_RESERVED_3 = 0x3,
    PAT_WRITE_THROUGH_WT = 0x4,
    PAT_WRITE_PROTECTED_WP = 0x5,
    PAT_WRITE_BACK_WB = 0x6,
    PAT_UNCACHED_UC_ = 0x7,
    PAT_NUMS
} PATEncoding;

static constexpr auto PAT_LOCATION = 0x277;

typedef struct {
    U8 pat : 3;
    U8 reserved : 5;
} PATEntry;

typedef struct {
    union {
        PATEntry pats[8];
        U64 value;
    };
} PAT;

static constexpr struct {
    U64 MAP_0;
    U64 MAP_1;
    U64 MAP_2;
    U64 MAP_3;
    U64 MAP_4;
    U64 MAP_5;
    U64 MAP_6;
    U64 MAP_7;
    U64 MAP_8;
} PATMapping = {.MAP_0 = 0,
                .MAP_1 = VirtualPageMasks.PAGE_WRITE_THROUGH,
                .MAP_2 = VirtualPageMasks.PAGE_DISABLE_CACHE,
                .MAP_3 = VirtualPageMasks.PAGE_WRITE_THROUGH |
                         VirtualPageMasks.PAGE_DISABLE_CACHE,
                // NOTE: The below PATs can NOT be used by extended sizes as the
                // CPU will think u set a large/huge page and select pat x - 4
                .MAP_4 = VirtualPageMasks.PAGE_EXTENDED_SIZE,
                .MAP_5 = VirtualPageMasks.PAGE_EXTENDED_SIZE |
                         VirtualPageMasks.PAGE_WRITE_THROUGH,
                .MAP_6 = VirtualPageMasks.PAGE_EXTENDED_SIZE |
                         VirtualPageMasks.PAGE_DISABLE_CACHE,
                .MAP_7 = VirtualPageMasks.PAGE_EXTENDED_SIZE |
                         VirtualPageMasks.PAGE_DISABLE_CACHE |
                         VirtualPageMasks.PAGE_WRITE_THROUGH};

#endif
