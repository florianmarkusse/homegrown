#include "image-builder/configuration.h"
#include "shared/memory/sizes.h"

// TODO: Move default LBA size to 4096 , seems better for performone on disks in
// this day and age
// And have it be dependent on or set it manually
// static U64 physicalBlockBoundary = 512;
// static U64 optimalTransferLengthGranularity = 512;
Configuration configuration = {.imageName = "new-image.hdd", .LBASize = 1024};

void setConfiguration() {
    // NOTE: this is just scaffolding, it should calculate the actual values
    // instead of just using hardcoded values.

    configuration.totalImageSizeLBA = SectionsInLBASize.PROTECTIVE_MBR;
    configuration.GPTPartitionTableSizeLBA =
        GPT_PARTITION_TABLE_SIZE / configuration.LBASize;
    configuration.alignmentLBA = (1 * MiB) / configuration.LBASize;
}
