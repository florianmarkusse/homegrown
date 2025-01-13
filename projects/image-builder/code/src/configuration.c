#include "image-builder/configuration.h"
#include "image-builder/partitions/efi.h"
#include "shared/maths/maths.h"
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

void setConfiguration(U64 efiApplicationSizeBytes) {
    srand((U32)time(nullptr));

    configuration.alignmentLBA = (U16)((1 * MiB) / configuration.LBASize);
    U32 currentLBA = 0;

    // MBR + primary GPT
    configuration.GPTPartitionTableSizeLBA =
        GPT_PARTITION_TABLE_SIZE / configuration.LBASize;
    currentLBA += SectionsInLBASize.PROTECTIVE_MBR +
                  SectionsInLBASize.GPT_HEADER +
                  configuration.GPTPartitionTableSizeLBA;
    currentLBA = ALIGN_UP_VALUE(currentLBA, configuration.alignmentLBA);

    // EFI Partition
    configuration.EFISystemPartitionStartLBA = currentLBA;
    configuration.EFISystemPartitionSizeLBA =
        calculateEFIPartitionSize((U32)CEILING_DIV_VALUE(
            efiApplicationSizeBytes, (U32)configuration.LBASize));
    currentLBA += configuration.EFISystemPartitionSizeLBA;
    currentLBA = ALIGN_UP_VALUE(currentLBA, configuration.alignmentLBA);

    // Data Partition
    configuration.DataPartitionStartLBA = currentLBA;
    configuration.DataPartitionSizeLBA = 16;
    currentLBA += configuration.DataPartitionSizeLBA;

    // Backup GPT
    currentLBA +=
        configuration.GPTPartitionTableSizeLBA + SectionsInLBASize.GPT_HEADER;
    configuration.totalImageSizeLBA = currentLBA;
    configuration.totalImageSizeBytes =
        configuration.totalImageSizeLBA * configuration.LBASize;
}
