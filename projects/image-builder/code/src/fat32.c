

#include "image-builder/configuration.h"
#include "shared/maths/maths.h"
#include "shared/types/types.h"
#include "uefi/constants.h"

// NOTE: I don't think anyone cares about these values?
typedef enum : U8 {
    REMOVABLE_MEDIA = 0xF0,
    FIXED_MEDIA = 0xF8,
} MediaValues;

static constexpr auto MIN_CLUSTERS_OF_FAT32 = 65525 + 1;

static constexpr U16 DEFAULT_RESERVED_SECTORS = 32;

static constexpr U8 INT_0X13_HARD_DISK_DRIVE_NUMBER = 0x80;

static constexpr U8 EXTENDED_BOOT_SIGNATURE = 0x29;

static constexpr U32 FILE_SYSTEM_INFO_UNKNOWN = 0xFFFFFFFF;

static constexpr U32 END_OF_CHAIN_MARKER = 0xFFFFFFFF;

// The rest of the logical sectors is zeroed out as well in these data structure
// blocks. The size of the sector is decided by BIOSParameter.bytesPerSector
// field.

typedef struct {
    U8 jmpBoot[3];
    U8 OEMName[8];
    U16 bytesPerSector;
    U8 sectorsPerCluster;
    U16 reservedSectors;
    U8 FATs;
    U16 rootEntries;
    U16 totalSectors16Bit;
    U8 media;
    U16 FATSize16;
    U16 sectorsPerTrack;
    U16 numberOfHeads;
    U32 hiddenSectors;
    U32 totalSectors32Bit;
    U32 FATSize32Sectors;
    U16 extendedFlags;
    U16 version;
    U32 rootClusterNumber;
    U16 FSInfoSector;
    U16 backupBootSector;
    U8 reserved[12];
    U8 driverNumber;
    U8 reserved1;
    U8 bootSignature;
    U8 volumeSerialNumber[4];
    U8 volumeLabel[11];
    U8 fileSystemType[8];
    U8 reserved2[90];
    U16 signature;
} BIOSParameterBlock;

typedef struct {
    U32 leadSignature;
    U8 reserved[480];
    U32 structureSignature;
    U32 freeCount;
    U32 nextFree;
    U8 reserved1[12];
    U32 trailSignature;
} FileSystemInformation;

static BIOSParameterBlock parameterBlock = {
    .jmpBoot = {0xEB, 0x00, 0x90},
    .OEMName = {"FATDISK "},
    .bytesPerSector = 0, // NOTE: Will be set at runtime
    .sectorsPerCluster = 1,
    .reservedSectors = DEFAULT_RESERVED_SECTORS,
    .FATs = 2,
    .rootEntries = 0,
    .totalSectors16Bit = 0,
    .media = FIXED_MEDIA,
    .FATSize16 = 0,
    .sectorsPerTrack = 0,
    .numberOfHeads = 0,
    .hiddenSectors = 0,     // NOTE: Will be set at runtime
    .totalSectors32Bit = 0, // NOTE: Will be set at runtime
    .FATSize32Sectors = 0,  // NOTE: Will be set at runtime

    .extendedFlags = 0, // Mirroring FATs, i.e., they are kept in sync.
    .version = 0,
    .rootClusterNumber = 2,
    .FSInfoSector = 1,
    .backupBootSector = 6,
    .reserved = {0},
    .driverNumber = INT_0X13_HARD_DISK_DRIVE_NUMBER,
    .reserved1 = 0,
    .bootSignature = EXTENDED_BOOT_SIGNATURE,
    .volumeSerialNumber = {0},
    .volumeLabel = {"NO NAME    "},
    .fileSystemType = {"FAT32   "},
    .reserved2 = {0},
    .signature = BOOT_SIGNATURE};

// Setting the values to unknown since we are not planning on reading from it
// and only writing to it once in a predetermined fashion. Hence, keepin track
// of the number and locations of free clusters is unnecessary.
static constexpr FileSystemInformation FSInfo = {
    .leadSignature = 0x41615252,
    .reserved = {0},
    .structureSignature = 0x61417272,
    .freeCount = FILE_SYSTEM_INFO_UNKNOWN,
    .nextFree = FILE_SYSTEM_INFO_UNKNOWN,
    .reserved1 = {0},
    .trailSignature = 0xAA550000};

U32 calculateSectorsPerFAT(U32 FATStartLBA, U32 dataRegionStartLBA) {
    U32 totalFatSectorsLBA = dataRegionStartLBA - FATStartLBA;
    U32 clustersPerFAT =
        (totalFatSectorsLBA / 2 * parameterBlock.bytesPerSector / sizeof(U32));
    return clustersPerFAT * parameterBlock.sectorsPerCluster;
}

void writeEFISystemPartition(U8 *fileBuffer, U8 *efiApplicationPath) {
    parameterBlock.bytesPerSector = configuration.LBASize;

    // We have to ensure that the total number of clusters of this volume is
    // large enough to be recognized as a FAT32 volume. The determination of FAT
    // type is completely done based on the number of total clusters.
    parameterBlock.totalSectors32Bit = configuration.EFISystemPartitionSizeLBA;
    if (parameterBlock.totalSectors32Bit * parameterBlock.sectorsPerCluster <
        MIN_CLUSTERS_OF_FAT32) {
        U32 requiredExtraSectors =
            MIN_CLUSTERS_OF_FAT32 - (parameterBlock.totalSectors32Bit *
                                     parameterBlock.sectorsPerCluster);
        parameterBlock.totalSectors32Bit += requiredExtraSectors;
    }

    U32 unalignedESPStartLBA = SectionsInLBASize.PROTECTIVE_MBR +
                               SectionsInLBASize.GPT_HEADER +
                               configuration.GPTPartitionTableSizeLBA;
    U32 alignedESPStartLBA =
        ALIGN_UP_VALUE(unalignedESPStartLBA, configuration.alignmentLBA);
    parameterBlock.hiddenSectors = alignedESPStartLBA - 1;

    U32 FATStartLBA = alignedESPStartLBA + parameterBlock.reservedSectors;

    U32 dataRegionStartLBA =
        ALIGN_UP_VALUE(FATStartLBA, configuration.alignmentLBA);
    U32 sectorsPerFAT = calculateSectorsPerFAT(FATStartLBA, dataRegionStartLBA);
    while (sectorsPerFAT < parameterBlock.totalSectors32Bit * 1.1) {
        dataRegionStartLBA =
            ALIGN_UP_VALUE(dataRegionStartLBA + 1, configuration.alignmentLBA);
        sectorsPerFAT = calculateSectorsPerFAT(FATStartLBA, dataRegionStartLBA);
    }
    parameterBlock.FATSize32Sectors = sectorsPerFAT;
}
