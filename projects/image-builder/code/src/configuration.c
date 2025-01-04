#include "image-builder/configuration.h"
#include "image-builder/partitions/efi.h"
#include "shared/memory/sizes.h"
#include <stdlib.h>
#include <time.h>

// TODO: Move default LBA size to 4096 , seems better for performone on disks in
// this day and age
// And have it be dependent on or set it manually
// static U64 physicalBlockBoundary = 512;
// static U64 optimalTransferLengthGranularity = 512;
// NOTE: minimum LBA size is 512! upwards with powers of 2
Configuration configuration = {.imageName = "image-builder-image.hdd",
                               .LBASize = 512};

void setConfiguration() {
    srand((U32)time(nullptr));

    configuration.alignmentLBA = (U16)((1 * MiB) / configuration.LBASize);

    // NOTE: this is just scaffolding, it should calculate the actual values
    // instead of just using hardcoded values.

    configuration.GPTPartitionTableSizeLBA =
        GPT_PARTITION_TABLE_SIZE / configuration.LBASize;
    // NOTE: this should just be based on the lba and other stuff and not
    // standard be this calculation

    configuration.EFISystemPartitionSizeLBA = calculateEFIPartitionSize(8);
    configuration.DataPartitionSizeLBA = 16;

    // Should be bigger than all other values, duh?
    configuration.totalImageSizeLBA = 200;
    configuration.totalImageSizeBytes =
        configuration.totalImageSizeLBA * configuration.LBASize;
}
