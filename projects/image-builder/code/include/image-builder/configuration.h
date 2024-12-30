#ifndef IMAGE_BUILDER_CONFIGURATION_H
#define IMAGE_BUILDER_CONFIGURATION_H

#include "shared/memory/sizes.h"
#include "shared/types/types.h"

// We are setting the number of entries to 128 and the size of each entry to 128
// too. This, from the spec can change in the future but I don't see how. For
// some reason, the GPT needs to specify this number of entries even if only a
// subset are actually used.
// Lastly, the minimum size of the table is 16KiB, which is also 128 * 128, so
// it all works out but it's a bit odd.
static constexpr auto GPT_PARTITION_TABLE_ENTRIES = 128;
static constexpr auto GPT_PARTITION_TABLE_ENTRY_SIZE = 128;
static constexpr auto GPT_PARTITION_TABLE_SIZE = 16 * KiB;

static constexpr struct {
    U16 PROTECTIVE_MBR;
    U16 GPT_HEADER;
} SectionsInLBASize = {
    .PROTECTIVE_MBR = 1,
    .GPT_HEADER = 1,
};

typedef struct {
    U8 *imageName;
    U16 LBASize;
    U32 alignmentLBA;
    U64 totalImageSizeLBA;
    U64 GPTPartitionTableSizeLBA;
    U64 EFISystemPartitionSizeLBA;
    U64 DataPartitionSizeLBA;
} Configuration;

extern Configuration configuration;

void setConfiguration();

#endif
