#include "image-builder/partitions/efi.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "image-builder/configuration.h"
#include "platform-abstraction/log.h"
#include "posix/log.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/macros.h"
#include "shared/maths/maths.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"
#include "uefi/constants.h"
#include "platform-abstraction/memory/manipulation.h"

// NOTE: This is a minimal FAT32 implementation. The following assumptions are
// made:
// - All names are using 8+3 short name.
// - At the start of initialization, the size given is enough for all the files
// that are supposed to be added.
// - All files and directories have different names. I.e., no checks are made
// for duplicate files/directories.
// - Directories will only contain as many children as fit into a single sector

static constexpr auto MIN_CLUSTERS_OF_FAT32 = 65525 + 1;
static constexpr U8 PATH_DELIMITER = '/';

static constexpr U16 DEFAULT_RESERVED_SECTORS = 32;

static constexpr U8 INT_0X13_HARD_DISK_DRIVE_NUMBER = 0x80;

static constexpr U8 EXTENDED_BOOT_SIGNATURE = 0x29;

typedef union {
    U32 full;
    struct {
        U16 low;
        U16 high;
    };
} Cluster;

// For end of file data or directory cluster
static constexpr U32 END_OF_CHAIN_MARKER = 0xFFFFFFFF;
static constexpr auto RESERVED_CLUSTER_INDICES = 2;
static constexpr Cluster ROOT_CLUSTER_FAT_INDEX = (Cluster){.full = 2};

constexpr static auto FAT32_SHORT_NAME_LEN = 11;
constexpr static U8 ZERO_NAME[FAT32_SHORT_NAME_LEN] = {0};

static U8 *DATA_CLUSTERS;
static U32 *PRIMARY_FAT;
static U32 DATA_CLUSTERS_COUNT;
static U32 FAT_SIZE_BYTES;
static Cluster CURRENT_FREE_DATA_CLUSTER_INDEX = (Cluster){.full = 0};

typedef enum : U8 {
    REMOVABLE_MEDIA = 0xF0,
    FIXED_MEDIA = 0xF8,
} MediaValues;

typedef enum : U8 {
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN = 0x02,
    ATTR_SYSTEM = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE = 0x20,
} DirectoryAttributes;

static constexpr DirectoryAttributes FAT32_FILE_ATTRIBUTES = 0;
static constexpr DirectoryAttributes FAT32_DIRECTORY_ATTRIBUTES =
    ATTR_DIRECTORY;

static constexpr struct {
    U8 DOT[FAT32_SHORT_NAME_LEN];
    U8 DOTDOT[FAT32_SHORT_NAME_LEN];
} StandardDirectory = {
    .DOT = ".          ",
    .DOTDOT = "..         ",
};

static constexpr struct {
    U8 DATA_FILE;
    U8 DISK_FILE;
} SectorSize = {.DATA_FILE = 1, .DISK_FILE = 1};

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
    U8 reserved2[420];
    U16 signature;
} __attribute__((packed)) BIOSParameterBlock;

typedef struct {
    U32 leadSignature;
    U8 reserved[480];
    U32 structureSignature;
    U32 freeCount;
    U32 nextFree;
    U8 reserved1[12];
    U32 trailSignature;
} __attribute__((packed)) FileSystemInformation;

typedef struct {
    U8 name[FAT32_SHORT_NAME_LEN];
    U8 attributes;
    U8 reserved;
    U8 CrtTimeTenth;
    U16 CrtTime;
    U16 CrtDate;
    U16 lastAccessDate;
    U16 highClusterNumber;
    U16 writeTime;
    U16 writeDate;
    U16 lowClusterNumber;
    U32 fileSize;
} __attribute__((packed)) DirEntryShortName;

static BIOSParameterBlock parameterBlock = {
    .jmpBoot = {0xEB, 0x00, 0x90},
    .OEMName = {"THISDISK"},
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

static FileSystemInformation FSInfo = {
    .leadSignature = 0x41615252,
    .reserved = {0},
    .structureSignature = 0x61417272,
    .freeCount = 0, // NOTE: Will be set at runtime
    .nextFree = 0,  // NOTE: Will be set at runtime
    .reserved1 = {0},
    .trailSignature = 0xAA550000};

U32 calculateEFIPartitionSize(U32 EFIApplicationSizeLBA) {
    // No need for conversions here because values are set to equal each other
    // 1 cluster == 1 sector == 1 LBA
    parameterBlock.bytesPerSector = configuration.LBASizeBytes;

    U32 requiredDataClusters =
        MAX(MIN_CLUSTERS_OF_FAT32,
            parameterBlock.sectorsPerCluster *
                (EFIApplicationSizeLBA + SectorSize.DATA_FILE +
                 SectorSize.DISK_FILE));
    DATA_CLUSTERS_COUNT =
        ALIGN_UP_VALUE(requiredDataClusters, configuration.alignmentLBA);

    U32 requiredFATSizeSectors = CEILING_DIV_VALUE(
        DATA_CLUSTERS_COUNT, (configuration.LBASizeBytes / (U32)sizeof(U32)));
    U32 reservedAndFATSizeSectors =
        ALIGN_UP_VALUE(parameterBlock.reservedSectors +
                           (requiredFATSizeSectors * parameterBlock.FATs),
                       configuration.alignmentLBA);

    parameterBlock.FATSize32Sectors =
        (reservedAndFATSizeSectors - parameterBlock.reservedSectors) /
        parameterBlock.FATs;
    FAT_SIZE_BYTES =
        parameterBlock.FATSize32Sectors * parameterBlock.bytesPerSector;
    parameterBlock.totalSectors32Bit =
        reservedAndFATSizeSectors + DATA_CLUSTERS_COUNT;

    return parameterBlock.totalSectors32Bit;
}

static void setName(DirEntryShortName *dir, U8 name[FAT32_SHORT_NAME_LEN]) {
    memcpy(dir->name, name, sizeof(((DirEntryShortName *)nullptr)->name));
}

static bool compareFAT32ShortName(U8 name1[FAT32_SHORT_NAME_LEN],
                                  U8 name2[FAT32_SHORT_NAME_LEN]) {
    return !memcmp(name1, name2, FAT32_SHORT_NAME_LEN);
}

static U8 *getDataCluster(Cluster clusterIndex) {
    return DATA_CLUSTERS + parameterBlock.bytesPerSector *
                               (clusterIndex.full - RESERVED_CLUSTER_INDICES);
}

typedef enum {
    CLUSTER_CHECK_SUCCESS,
    CLUSTER_CHECK_DUPLICATE_NAME,
} ClusterCheckResult;

typedef struct {
    U8_a remainingSpace;
    ClusterCheckResult result;
} ClusterCheck;

static void writeDirectoryEntry(DirEntryShortName *clusterBuffer,
                                Cluster cluster,
                                U8 name[FAT32_SHORT_NAME_LEN]) {
    static DirEntryShortName dir = (DirEntryShortName){
        .attributes = FAT32_DIRECTORY_ATTRIBUTES,
    };

    dir.highClusterNumber = cluster.high;
    dir.lowClusterNumber = cluster.low;
    setName(&dir, name);

    memcpy(clusterBuffer, &dir, sizeof(DirEntryShortName));
}

static void writeDOTEntries(DirEntryShortName *clusterBuffer,
                            Cluster currentCluster, Cluster parentCluster) {
    writeDirectoryEntry(clusterBuffer, currentCluster, StandardDirectory.DOT);
    writeDirectoryEntry(clusterBuffer + 1, parentCluster,
                        StandardDirectory.DOTDOT);
}

static Cluster createContiguousSpaceForNewEntry(U64 bytes) {
    ASSERT(bytes > 0);

    U32 requiredNewClusters =
        (U32)CEILING_DIV_VALUE(bytes, (U32)parameterBlock.bytesPerSector);
    ASSERT(requiredNewClusters <
           (DATA_CLUSTERS_COUNT - CURRENT_FREE_DATA_CLUSTER_INDEX.full));

    Cluster result = CURRENT_FREE_DATA_CLUSTER_INDEX;

    U32 exclusiveLastCluster =
        CURRENT_FREE_DATA_CLUSTER_INDEX.full + requiredNewClusters;
    U32 lastCluster = exclusiveLastCluster - 1;
    for (U32 i = CURRENT_FREE_DATA_CLUSTER_INDEX.full; i < exclusiveLastCluster;
         i++) {
        if (i == lastCluster) {
            PRIMARY_FAT[i] = END_OF_CHAIN_MARKER;
        } else {
            PRIMARY_FAT[i] = i + 1;
        }
    }

    CURRENT_FREE_DATA_CLUSTER_INDEX.full = exclusiveLastCluster;

    return result;
}

static Cluster createOrGetDirectory(U8 name[FAT32_SHORT_NAME_LEN],
                                    Cluster parentCluster) {
    U8 *dataClusterLocation = getDataCluster(parentCluster);
    U8 *dataClusterEnd = dataClusterLocation + parameterBlock.bytesPerSector;
    for (DirEntryShortName *entry = (DirEntryShortName *)dataClusterLocation;
         (U8 *)entry < dataClusterEnd; entry++) {
        if (compareFAT32ShortName(entry->name, name)) {
            return (Cluster){.low = entry->lowClusterNumber,
                             .high = entry->highClusterNumber};
        }
        if (compareFAT32ShortName(entry->name, ZERO_NAME)) {
            Cluster createdCluster =
                createContiguousSpaceForNewEntry(sizeof(DirEntryShortName));
            writeDirectoryEntry(entry, createdCluster, name);
            writeDOTEntries((DirEntryShortName *)getDataCluster(createdCluster),
                            createdCluster, parentCluster);

            return createdCluster;
        }
    }

    __builtin_unreachable();
}

static DirEntryShortName *getNewFileEntryLocation(Cluster parentCluster) {
    U8 *dataClusterLocation = getDataCluster(parentCluster);
    U8 *dataClusterEnd = dataClusterLocation + parameterBlock.bytesPerSector;
    for (DirEntryShortName *entry = (DirEntryShortName *)dataClusterLocation;
         (U8 *)entry < dataClusterEnd; entry++) {
        if (compareFAT32ShortName(entry->name, ZERO_NAME)) {
            return entry;
        }
    }

    __builtin_unreachable();
}

static void writeFileEntry(DirEntryShortName *clusterBuffer,
                           U8 name[FAT32_SHORT_NAME_LEN], U32 size) {
    static DirEntryShortName file = (DirEntryShortName){
        .attributes = FAT32_FILE_ATTRIBUTES,
    };
    file.highClusterNumber = CURRENT_FREE_DATA_CLUSTER_INDEX.high;
    file.lowClusterNumber = CURRENT_FREE_DATA_CLUSTER_INDEX.low;
    setName(&file, name);
    file.fileSize = size;

    memcpy(clusterBuffer, &file, sizeof(DirEntryShortName));
}

static void createFileEntryAfterWritingData(U8 name[FAT32_SHORT_NAME_LEN],
                                            Cluster parentCluster, U32 size) {
    writeFileEntry(getNewFileEntryLocation(parentCluster), name, size);
    createContiguousSpaceForNewEntry(size);
}

static Cluster createPath(string FAT32FilePath, Cluster startCluster) {
    StringIter parentDirectories;

    TOKENIZE_STRING(FAT32FilePath, parentDirectories, PATH_DELIMITER, 0) {
        startCluster =
            createOrGetDirectory(parentDirectories.string.buf, startCluster);
    }

    return startCluster;
}

#define CREATE_FAT32_FILE(fileName, parentCluster, bufferVariable,             \
                          bufferStartVariable)                                 \
    (bufferVariable) = getDataCluster(CURRENT_FREE_DATA_CLUSTER_INDEX);        \
    (bufferStartVariable) = (bufferVariable);                                  \
    for (auto MACRO_VAR(i) = 0; MACRO_VAR(i) < 1; MACRO_VAR(i) = 1,            \
              createFileEntryAfterWritingData((fileName), parentCluster,       \
                                              (U32)((bufferVariable) -         \
                                                    (bufferStartVariable))))

bool writeEFISystemPartition(U8 *fileBuffer, int efifd, U64 efiSizeBytes,
                             U64 kernelSizeBytes) {
    parameterBlock.hiddenSectors = configuration.EFISystemPartitionStartLBA;

    // We write the file data first and later fill the reserved sectors. The
    // order is reversed because this way we can set the value of file system
    // info sector correctly and only need to do it once.

    fileBuffer +=
        configuration.EFISystemPartitionStartLBA * configuration.LBASizeBytes;
    U8 *reservedSectors = fileBuffer;

    fileBuffer +=
        parameterBlock.bytesPerSector * parameterBlock.reservedSectors;
    PRIMARY_FAT = (U32 *)fileBuffer;

    // Reserved entries
    PRIMARY_FAT[CURRENT_FREE_DATA_CLUSTER_INDEX.full] =
        0xFFFFFF00 | parameterBlock.media;
    CURRENT_FREE_DATA_CLUSTER_INDEX.full++;
    PRIMARY_FAT[CURRENT_FREE_DATA_CLUSTER_INDEX.full] = END_OF_CHAIN_MARKER;
    CURRENT_FREE_DATA_CLUSTER_INDEX.full++;
    // Root directory
    PRIMARY_FAT[CURRENT_FREE_DATA_CLUSTER_INDEX.full] = END_OF_CHAIN_MARKER;
    CURRENT_FREE_DATA_CLUSTER_INDEX.full++;

    DATA_CLUSTERS =
        ((U8 *)PRIMARY_FAT) + (FAT_SIZE_BYTES * parameterBlock.FATs);

    Cluster efiCluster =
        createPath(STRING("EFI        "), ROOT_CLUSTER_FAT_INDEX);
    Cluster efiBootCluster = createPath(STRING("BOOT       "), efiCluster);
    Cluster efiFLOSCluster = createPath(STRING("FLOS       "), efiCluster);

    U8 *FAT32FileBuffer;
    U8 *dataStartLocation;

    CREATE_FAT32_FILE("BOOTX64 EFI", efiBootCluster, FAT32FileBuffer,
                      dataStartLocation) {
        for (U8 *exclusiveEnd = FAT32FileBuffer + efiSizeBytes;
             FAT32FileBuffer < exclusiveEnd;) {
            I64 partialBytesRead = read(efifd, FAT32FileBuffer,
                                        (U64)(exclusiveEnd - FAT32FileBuffer));
            if (partialBytesRead < 0) {
                ASSERT(false);
                PFLUSH_AFTER(STDERR) {
                    PERROR((STRING("Failed to read bytes from efi file to "
                                   "write to ESP!\n")));
                    PERROR(STRING("Error code: "));
                    PERROR(errno, NEWLINE);
                    PERROR(STRING("Error message: "));
                    U8 *errorString = strerror(errno);
                    PERROR(STRING_LEN(errorString, strlen(errorString)),
                           NEWLINE);
                }
                return false;
            } else {
                FAT32FileBuffer += partialBytesRead;
            }
        }
    }

    CREATE_FAT32_FILE("KERNEL  INF", efiFLOSCluster, FAT32FileBuffer,
                      dataStartLocation) {
        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, STRING("KERNEL_SIZE_BYTES="));
        FAT32FileBuffer += KLOG_APPEND(FAT32FileBuffer, kernelSizeBytes);
        FAT32FileBuffer += KLOG_APPEND(FAT32FileBuffer, STRING("\n"));

        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, STRING("KERNEL_START_LBA="));
        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, configuration.dataPartitionStartLBA);
        FAT32FileBuffer += KLOG_APPEND(FAT32FileBuffer, STRING("\n"));
    }

    CREATE_FAT32_FILE("DISKDATAINF", efiFLOSCluster, FAT32FileBuffer,
                      dataStartLocation) {
        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, STRING("DISK_SIZE_BYTES="));
        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, configuration.totalImageSizeBytes);
        FAT32FileBuffer += KLOG_APPEND(FAT32FileBuffer, STRING("\n"));

        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, STRING("DISK_SIZE_LBA="));
        FAT32FileBuffer +=
            KLOG_APPEND(FAT32FileBuffer, configuration.totalImageSizeLBA);
        FAT32FileBuffer += KLOG_APPEND(FAT32FileBuffer, STRING("\n"));
    }

    U32 *mirrorLocation = PRIMARY_FAT + FAT_SIZE_BYTES;
    for (U8 i = 0; i < parameterBlock.FATs - 1; i++) {
        memcpy(mirrorLocation, PRIMARY_FAT, FAT_SIZE_BYTES);
        mirrorLocation += FAT_SIZE_BYTES;
    }

    FSInfo.freeCount =
        DATA_CLUSTERS_COUNT - CURRENT_FREE_DATA_CLUSTER_INDEX.full;
    FSInfo.nextFree = CURRENT_FREE_DATA_CLUSTER_INDEX.full;

    // Reserved Sectors
    // Primary Sectors
    // Sector 0
    memcpy(reservedSectors, &parameterBlock, sizeof(BIOSParameterBlock));

    // Sector 1
    reservedSectors += parameterBlock.bytesPerSector;
    memcpy(reservedSectors, &FSInfo, sizeof(FileSystemInformation));

    // Backup Sectors
    // Sector 6
    reservedSectors += parameterBlock.bytesPerSector * 5;
    memcpy(reservedSectors, &parameterBlock, sizeof(BIOSParameterBlock));
    // Sector 7
    reservedSectors += parameterBlock.bytesPerSector;
    memcpy(reservedSectors, &FSInfo, sizeof(FileSystemInformation));

    return true;
}
