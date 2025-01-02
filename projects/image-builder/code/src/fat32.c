

#include "shared/types/types.h"

static constexpr U32 UNKNOWN = 0xFFFFFFFF;
static constexpr U32 LAST_CLUSTER_IN_FILE = 0xFFFFFFFF;

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
    U32 FATSize32;
    U16 extendedFlags;
    U16 version;
    U32 rootClusterNumber;
    U16 FSInfoSector;
    U16 bootSector;
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
