#include "image-builder/configuration.h"

#include <string.h>

#include "image-builder/partitions/efi.h"
#include "abstraction/log.h"
#include "posix/log.h"
#include "shared/log.h"
#include "shared/maths/maths.h"
#include "shared/text/string.h"

Configuration configuration = {
    .imageName = "FLOS_UEFI_IMAGE.hdd", .LBASizeBytes = 512, .alignmentLBA = 1};

void setConfiguration(U64 efiApplicationSizeBytes, U64 kernelSizeBytes,
                      U32 alignmentSizeBytes) {
    if (alignmentSizeBytes > configuration.LBASizeBytes) {
        configuration.alignmentLBA =
            alignmentSizeBytes / configuration.LBASizeBytes;
    }
    U32 currentLBA = 0;

    // MBR + primary GPT
    configuration.GPTPartitionTableSizeLBA =
        GPT_PARTITION_TABLE_SIZE / configuration.LBASizeBytes;
    currentLBA += SectionsInLBASize.PROTECTIVE_MBR +
                  SectionsInLBASize.GPT_HEADER +
                  configuration.GPTPartitionTableSizeLBA;
    currentLBA = ALIGN_UP_VALUE(currentLBA, configuration.alignmentLBA);

    // EFI Partition
    configuration.EFISystemPartitionStartLBA = currentLBA;
    U32 unalignedLBA = calculateEFIPartitionSize((U32)CEILING_DIV_VALUE(
        efiApplicationSizeBytes, (U32)configuration.LBASizeBytes));
    configuration.EFISystemPartitionSizeLBA =
        ALIGN_UP_VALUE(unalignedLBA, configuration.alignmentLBA);
    currentLBA += configuration.EFISystemPartitionSizeLBA;

    // Data Partition
    configuration.dataPartitionStartLBA = currentLBA;
    configuration.dataPartitionSizeLBA = (U32)CEILING_DIV_VALUE(
        kernelSizeBytes, (U32)configuration.LBASizeBytes);
    currentLBA += configuration.dataPartitionSizeLBA;

    // Backup GPT
    currentLBA +=
        configuration.GPTPartitionTableSizeLBA + SectionsInLBASize.GPT_HEADER;
    configuration.totalImageSizeLBA = currentLBA;
    configuration.totalImageSizeBytes =
        configuration.totalImageSizeLBA * configuration.LBASizeBytes;

    PFLUSH_AFTER(STDOUT) {
        INFO(STRING("Configuration\n"));

        INFO(STRING("Image name: "));
        INFO(STRING_LEN(configuration.imageName,
                        strlen(configuration.imageName)),
             NEWLINE);

        INFO(STRING("LBA size bytes: "));
        INFO(configuration.LBASizeBytes, NEWLINE);

        INFO(STRING("Alignment in LBA: "));
        INFO(configuration.alignmentLBA, NEWLINE);

        INFO(STRING("total image size LBA: "));
        INFO(configuration.totalImageSizeLBA, NEWLINE);

        INFO(STRING("total image size bytes: "));
        INFO(configuration.totalImageSizeBytes, NEWLINE);

        INFO(STRING("GPT partition table size LBA: "));
        INFO(configuration.GPTPartitionTableSizeLBA, NEWLINE);

        INFO(STRING("EFI partition start LBA: "));
        INFO(configuration.EFISystemPartitionStartLBA, NEWLINE);

        INFO(STRING("EFI partition size LBA: "));
        INFO(configuration.EFISystemPartitionSizeLBA, NEWLINE);

        INFO(STRING("Data partition start LBA: "));
        INFO(configuration.dataPartitionStartLBA, NEWLINE);

        INFO(STRING("Data partition size LBA: "));
        INFO(configuration.dataPartitionSizeLBA, NEWLINE);
    }
}
