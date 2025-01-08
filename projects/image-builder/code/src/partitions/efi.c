#include "image-builder/partitions/efi.h"
#include "image-builder/configuration.h"
#include "platform-abstraction/log.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/maths/maths.h"
#include "shared/text/converter.h"
#include "shared/text/string.h"
#include "shared/types/types.h"
#include "uefi/constants.h"

static constexpr auto MIN_CLUSTERS_OF_FAT32 = 65525 + 1;

static constexpr U16 DEFAULT_RESERVED_SECTORS = 32;

static constexpr U8 INT_0X13_HARD_DISK_DRIVE_NUMBER = 0x80;

static constexpr U8 EXTENDED_BOOT_SIGNATURE = 0x29;

static constexpr U32 FILE_SYSTEM_INFO_UNKNOWN = 0xFFFFFFFF;

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
static Cluster CURRENT_FREE_DATA_CLUSTER_INDEX = (Cluster){.full = 0};

// NOTE: I don't think anyone cares about these values?
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
    U8 reserved2[90];
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

U32 calculateEFIPartitionSize(U32 EFIApplicationSizeLBA) {
    // No need for conversions here because values are set to equal each other
    // 1 cluster == 1 sector == 1 LBA
    parameterBlock.bytesPerSector = configuration.LBASize;

    U32 requiredDataClusters =
        MAX(MIN_CLUSTERS_OF_FAT32,
            parameterBlock.sectorsPerCluster *
                (EFIApplicationSizeLBA + SectorSize.DATA_FILE +
                 SectorSize.DISK_FILE));
    DATA_CLUSTERS_COUNT =
        ALIGN_UP_VALUE(requiredDataClusters, configuration.alignmentLBA);

    U32 requiredFATSizeSectors = CEILING_DIV_VALUE(
        DATA_CLUSTERS_COUNT, (configuration.LBASize / (U32)sizeof(U32)));
    U32 reservedAndFATSizeSectors =
        ALIGN_UP_VALUE(parameterBlock.reservedSectors +
                           (requiredFATSizeSectors * parameterBlock.FATs),
                       configuration.alignmentLBA);

    parameterBlock.FATSize32Sectors =
        (reservedAndFATSizeSectors - parameterBlock.reservedSectors) /
        parameterBlock.FATs;
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

static U32 nextWrittenCluster(U32 previousClusterIndex) {
    U32 index = previousClusterIndex;
    while (PRIMARY_FAT[index] != END_OF_CHAIN_MARKER) {
        index = PRIMARY_FAT[index];
    }
    return index;
}

// TODO: change to actually be the function that just writes the data into the
// new data clusters too
static U8 *allocateNewDataCluster(U32 currentLastClusterIndex, U32 size) {
    ASSERT(size > 0);

    U32 requiredNewClusters =
        CEILING_DIV_VALUE(size, (U32)parameterBlock.bytesPerSector);
    if (requiredNewClusters >
        (DATA_CLUSTERS_COUNT - CURRENT_FREE_DATA_CLUSTER_INDEX.full)) {
        return nullptr;
    }
    PRIMARY_FAT[currentLastClusterIndex] = CURRENT_FREE_DATA_CLUSTER_INDEX.full;
    U8 *result = getDataCluster(CURRENT_FREE_DATA_CLUSTER_INDEX);
    requiredNewClusters--;

    U32 lastAllocatedCluster =
        CURRENT_FREE_DATA_CLUSTER_INDEX.full + requiredNewClusters;
    for (U32 i = CURRENT_FREE_DATA_CLUSTER_INDEX.full; i < lastAllocatedCluster;
         i++) {
        PRIMARY_FAT[i] = PRIMARY_FAT[i + 1];
    }

    PRIMARY_FAT[lastAllocatedCluster] = END_OF_CHAIN_MARKER;

    CURRENT_FREE_DATA_CLUSTER_INDEX.full = lastAllocatedCluster + 1;

    return result;
}

typedef enum {
    CLUSTER_CHECK_SUCCESS,
    CLUSTER_CHECK_DUPLICATE_NAME,
} ClusterCheckResult;

typedef struct {
    U8_a remainingSpace;
    ClusterCheckResult result;
} ClusterCheck;

#define TRAVERSE_DATA_CLUSTERS(clusterStartPointer, clusterIndex)              \
    for ((clusterStartPointer) = getDataCluster(clusterIndex);;                \
         (clusterIndex).full = PRIMARY_FAT[(clusterIndex).full])

#define TRAVERSE_CLUSTER_ENTRIES(entryPointer, clusterEndPointer)              \
    for ((clusterEndPointer) = (entryPointer) + parameterBlock.bytesPerSector; \
         (entryPointer) < (clusterEndPointer);                                 \
         (entryPointer) += sizeof(DirEntryShortName))

static ClusterCheck checkAllocatedClusters(Cluster currentClusterIndex,
                                           U8 name[FAT32_SHORT_NAME_LEN]) {
    U8 *entryLocation;
    TRAVERSE_DATA_CLUSTERS(entryLocation, currentClusterIndex) {
        U8 *dataClusterEnd;
        TRAVERSE_CLUSTER_ENTRIES(entryLocation, dataClusterEnd) {
            U8 *nameAtLocation = ((DirEntryShortName *)entryLocation)->name;
            if (compareFAT32ShortName(nameAtLocation, ZERO_NAME)) {
                return (ClusterCheck){
                    .remainingSpace =
                        (U8_a){.buf = entryLocation,
                               .len = dataClusterEnd - entryLocation},
                    .result = CLUSTER_CHECK_SUCCESS};
            }
            if (compareFAT32ShortName(nameAtLocation, name)) {
                return (ClusterCheck){.result = CLUSTER_CHECK_DUPLICATE_NAME};
            }
        }

        if (PRIMARY_FAT[currentClusterIndex.full] == END_OF_CHAIN_MARKER) {
            return (ClusterCheck){.result = CLUSTER_CHECK_SUCCESS};
        }
    }
    /*for (U8 *possibleWriteLocation = getDataCluster(currentClusterIndex);;*/
    /*     currentClusterIndex = PRIMARY_FAT[currentClusterIndex]) {*/
    /*    for (U8 *dataClusterEnd =*/
    /*             possibleWriteLocation + parameterBlock.bytesPerSector;*/
    /*         possibleWriteLocation < dataClusterEnd;*/
    /*         possibleWriteLocation += sizeof(DirEntryShortName)) {*/
    /*        U8 *nameAtLocation =*/
    /*            ((DirEntryShortName *)possibleWriteLocation)->name;*/
    /*        if (compareFAT32ShortName(nameAtLocation, ZERO_NAME)) {*/
    /*            return (ClusterCheck){*/
    /*                .remainingSpace =*/
    /*                    (U8_a){.buf = possibleWriteLocation,*/
    /*                           .len = dataClusterEnd -
     * possibleWriteLocation},*/
    /*                .result = CLUSTER_CHECK_SUCCESS};*/
    /*        }*/
    /*        if (compareFAT32ShortName(nameAtLocation, name)) {*/
    /*            return (ClusterCheck){.result =
     * CLUSTER_CHECK_DUPLICATE_NAME};*/
    /*        }*/
    /*    }*/
    /**/
    /*    if (PRIMARY_FAT[currentClusterIndex] == END_OF_CHAIN_MARKER) {*/
    /*        return (ClusterCheck){.result = CLUSTER_CHECK_SUCCESS};*/
    /*    }*/
    /*}*/
}

static U8 *writeFileEntryAndAdvance(U8 *clusterBuffer, Cluster cluster,
                                    U8 name[FAT32_SHORT_NAME_LEN], U32 size) {
    static DirEntryShortName file = {0};
    file.highClusterNumber = cluster.high;
    file.lowClusterNumber = cluster.low;
    setName(&file, name);
    file.fileSize = size;

    memcpy(clusterBuffer, &file, sizeof(DirEntryShortName));

    return clusterBuffer + sizeof(DirEntryShortName);
}

static U8 *writeDirectoryEntryAndAdvance(U8 *clusterBuffer, Cluster cluster,
                                         U8 name[FAT32_SHORT_NAME_LEN]) {
    static DirEntryShortName dir = (DirEntryShortName){
        .attributes = ATTR_DIRECTORY,
    };

    dir.highClusterNumber = cluster.high;
    dir.lowClusterNumber = cluster.low;
    setName(&dir, name);

    memcpy(clusterBuffer, &dir, sizeof(DirEntryShortName));

    return clusterBuffer + sizeof(DirEntryShortName);
}

static U8 *writeDOTEntriesAndAdvance(U8 *clusterBuffer, Cluster currentCluster,
                                     Cluster parentCluster) {
    clusterBuffer = writeDirectoryEntryAndAdvance(clusterBuffer, currentCluster,
                                                  StandardDirectory.DOT);
    clusterBuffer = writeDirectoryEntryAndAdvance(clusterBuffer, parentCluster,
                                                  StandardDirectory.DOTDOT);

    return clusterBuffer;
}

static Cluster findClusterIndexStart(U8 name[FAT32_SHORT_NAME_LEN],
                                     Cluster currentClusterIndex) {
    U8 *entryLocation;
    TRAVERSE_DATA_CLUSTERS(entryLocation, currentClusterIndex) {
        U8 *dataClusterEnd;
        TRAVERSE_CLUSTER_ENTRIES(entryLocation, dataClusterEnd) {
            DirEntryShortName *entry = ((DirEntryShortName *)entryLocation);
            if (compareFAT32ShortName(entry->name, name)) {
                return (Cluster){.low = entry->lowClusterNumber,
                                 .high = entry->highClusterNumber};
            }
        }
    }

    return (Cluster){0};
}

static U8 addDirectory(string FAT32FilePath, string directoryFAT32) {
    Cluster currentCluster = ROOT_CLUSTER_FAT_INDEX;

    // TODO: add file/dir name check when finding new cluster, so need to
    // actually traverse all clusters of a directory to check for that :)
    // TODO: work on traversing the directories here!
    StringIter parentDirectories;
    TOKENIZE_STRING(FAT32FilePath, parentDirectories, '/', 0) {
        Cluster newCluster =
            findClusterIndexStart(parentDirectories.string.buf, currentCluster);
        if (!newCluster.full) {
            // create thi
        }

        currentCluster = newCluster;
    }
    return 0;
}

static void addFile(string FAT32FilePath, string fileFAT32) {
    U8 *rootDataCluster = getDataCluster(ROOT_CLUSTER_FAT_INDEX);

    StringIter parentDirectories;
    TOKENIZE_STRING(FAT32FilePath, parentDirectories, '/', 0) {
        if (parentDirectories.string.len) {
        }
    }
}

void writeEFISystemPartition(U8 *fileBuffer, U8 *efiApplicationPath) {
    parameterBlock.hiddenSectors = configuration.EFISystemPartitionStartLBA;

    // Reserved Sectors
    // Sector 0
    fileBuffer +=
        configuration.EFISystemPartitionStartLBA * configuration.LBASize;
    memcpy(fileBuffer, &parameterBlock, sizeof(BIOSParameterBlock));

    // Sector 1
    fileBuffer += parameterBlock.bytesPerSector;
    memcpy(fileBuffer, &FSInfo, sizeof(FileSystemInformation));

    // Sector 6
    fileBuffer += parameterBlock.bytesPerSector * 5;
    memcpy(fileBuffer, &parameterBlock, sizeof(BIOSParameterBlock));
    // Sector 7
    fileBuffer += parameterBlock.bytesPerSector;
    memcpy(fileBuffer, &FSInfo, sizeof(FileSystemInformation));

    // FAT
    fileBuffer +=
        parameterBlock.bytesPerSector * (parameterBlock.reservedSectors - 7);
    PRIMARY_FAT = (U32 *)fileBuffer;

    // We are assuming the following here:
    // - All the directories' entries fit into a single cluster
    // - Auxiliary files each fit into a singel cluster
    // So, we can just fill the clusters sequentially until the boot
    // file which can contain more than a single cluster
    static constexpr U32 ROOT_DIRECTORY_CLUSTER = 2;
    /*static constexpr U32 EFI_DIRECTORY_CLUSTER = 3;*/
    /*static constexpr U32 EFI_BOOT_DIRECTORY_CLUSTER = 4;*/
    /*static constexpr U32 EFI_FLOS_DIRECTORY_CLUSTER = 5;*/
    /*static constexpr U32 EFI_FLOS_KERNEL_INF_CLUSTER = 6;*/
    /*static constexpr U32 EFI_FLOS_DISKDATA_INF_CLUSTER = 7;*/
    /*static constexpr U32 EFI_BOOT_BOOTX64_EFI_CLUSTER = 8;*/

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
        fileBuffer + (parameterBlock.FATSize32Sectors *
                      parameterBlock.bytesPerSector * parameterBlock.FATs);

    addDirectory(EMPTY_STRING, STRING("EFI        "));
    addDirectory(STRING("/EFI        /BOOT       /NUTTYDICKXX"),
                 STRING("EFI        "));

    addFile(STRING("/EFI        /BOOT       "), STRING("BOOTX64 EFI"));

    for (U8 i = 0; i < parameterBlock.FATs - 1; i++) {
        memcpy(fileBuffer, PRIMARY_FAT,
               parameterBlock.FATSize32Sectors * parameterBlock.bytesPerSector);
        fileBuffer +=
            parameterBlock.FATSize32Sectors * parameterBlock.bytesPerSector;
    }

    /*FAT[EFI_DIRECTORY_CLUSTER] = END_OF_CHAIN_MARKER;*/
    /*FAT[EFI_BOOT_DIRECTORY_CLUSTER] = END_OF_CHAIN_MARKER;*/
    /*FAT[EFI_FLOS_DIRECTORY_CLUSTER] = END_OF_CHAIN_MARKER;*/
    /*FAT[EFI_FLOS_KERNEL_INF_CLUSTER] = END_OF_CHAIN_MARKER;*/
    /*FAT[EFI_FLOS_DISKDATA_INF_CLUSTER] = END_OF_CHAIN_MARKER;*/
    /**/
    /*U32 EFIApplicationSizeBytes = configuration.LBASize * 1024;*/
    /*U32 EFIApplicationRequiredClusters = CEILING_DIV_VALUE(*/
    /*    EFIApplicationSizeBytes + (U32)sizeof(DirEntryShortName),*/
    /*    (U32)parameterBlock.bytesPerSector);*/
    /*for (U32 i = 0; i < EFIApplicationRequiredClusters; i++) {*/
    /*    if (i == EFIApplicationRequiredClusters - 1) {*/
    /*        FAT[EFI_BOOT_BOOTX64_EFI_CLUSTER + i] =
     * END_OF_CHAIN_MARKER;*/
    /*    } else {*/
    /*        FAT[EFI_BOOT_BOOTX64_EFI_CLUSTER + i] =*/
    /*            EFI_BOOT_BOOTX64_EFI_CLUSTER + i + 1;*/
    /*    }*/
    /*}*/
    /**/
    /*fileBuffer +=*/
    /*    parameterBlock.FATSize32Sectors *
     * parameterBlock.bytesPerSector;*/
    /**/
    /*// Takes care of the mirroring*/
    /*for (U8 i = 0; i < parameterBlock.FATs - 1; i++) {*/
    /*    memcpy(fileBuffer, FAT,*/
    /*           parameterBlock.FATSize32Sectors *
     * parameterBlock.bytesPerSector);*/
    /*    fileBuffer +=*/
    /*        parameterBlock.FATSize32Sectors *
     * parameterBlock.bytesPerSector;*/
    /*}*/
    /**/
    /*// Data Sectors*/
    /*//  -Root Directory entries:*/
    /*//      - /EFI*/
    /*writeDirectoryEntryAndAdvance(fileBuffer, EFI_DIRECTORY_CLUSTER,*/
    /*                              "EFI        ");*/
    /**/
    /*// /EFI entries:*/
    /*//  - .*/
    /*//  - ..*/
    /*//  - /BOOT*/
    /*//  - /FLOS*/
    /*fileBuffer += parameterBlock.bytesPerSector;*/
    /*U8 *clusterBuffer = writeDOTEntriesAndAdvance(*/
    /*    fileBuffer, EFI_DIRECTORY_CLUSTER, ROOT_CLUSTER_AS_PARENT);*/
    /*clusterBuffer = writeDirectoryEntryAndAdvance(*/
    /*    clusterBuffer, EFI_BOOT_DIRECTORY_CLUSTER, "BOOT       ");*/
    /*clusterBuffer = writeDirectoryEntryAndAdvance(*/
    /*    clusterBuffer, EFI_FLOS_DIRECTORY_CLUSTER, "FLOS       ");*/
    /**/
    /*// /EFI/BOOT entries:*/
    /*//  - .*/
    /*//  - ..*/
    /*//  - /BOOTX64.EFI*/
    /*fileBuffer += parameterBlock.bytesPerSector;*/
    /**/
    /*// /EFI/FLOS entries:*/
    /*//  - .*/
    /*//  - ..*/
    /*//  - /KERNEL.INF*/
    /*//  - /DISKDATA.INF*/
    /*fileBuffer += parameterBlock.bytesPerSector;*/
    /*clusterBuffer = writeDOTEntriesAndAdvance(*/
    /*    fileBuffer, EFI_FLOS_DIRECTORY_CLUSTER,
     * EFI_DIRECTORY_CLUSTER);*/
    /*// TODO: We should probably flip this, write the data first and
     * then the*/
    /*// entry I think for these ad-hoc created files where we don't
     * know the size*/
    /*// in bytes beforehand.*/
    /*U32 KERNELINFSizeBytes = 256;*/
    /*clusterBuffer =*/
    /*    writeFileEntryAndAdvance(clusterBuffer,
     * EFI_FLOS_KERNEL_INF_CLUSTER,*/
    /*                             "KERNEL  INF", KERNELINFSizeBytes);*/
    /*clusterBuffer += KLOG_APPEND(clusterBuffer, 16384);*/
    /*clusterBuffer += KLOG_APPEND(clusterBuffer, STRING("\n"));*/
    /*clusterBuffer += KLOG_APPEND(clusterBuffer, 10000);*/
    /**/
    /*U32 DISKDATASizeBytes = 128;*/
    /*clusterBuffer =*/
    /*    writeFileEntryAndAdvance(clusterBuffer,
     * EFI_FLOS_DISKDATA_INF_CLUSTER,*/
    /*                             "DISKDATAINF", DISKDATASizeBytes);*/
}
