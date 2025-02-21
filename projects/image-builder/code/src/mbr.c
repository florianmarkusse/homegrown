#include "image-builder/mbr.h"
#include "abstraction/memory/manipulation.h"
#include "efi/uefi.h"
#include "image-builder/configuration.h"
#include "shared/types/types.h"

typedef struct {
    U8 bootIndicator;
    U8 startingCHS[3];
    U8 osType;
    U8 endingCHS[3];
    U32 startingLBA;
    U32 sizeLBA;
} __attribute__((packed)) MBRPartition;

typedef struct {
    U8 bootCode[440];
    U32 MBRSignature;
    U16 unknown;
    MBRPartition partitions[4];
    U16 signature;
} __attribute__((packed)) MBR;

static MBR protectiveMBR = {
    .bootCode = {0},
    .MBRSignature = 0,
    .unknown = 0,
    .partitions =
        {
            {
                .bootIndicator = 0,
                .startingCHS = {0x00, 0x02, 0x00},
                .osType = 0xEE,
                .endingCHS = {0xFF, 0xFF, 0xFF},
                .startingLBA = 0x00000001,
                .sizeLBA = 0, // NOTE: Will be set at runtime
            },
            {0},
            {0},
            {0},
        },
    .signature = BOOT_SIGNATURE,
};

void writeMBR(U8 *fileBuffer) {
    U64 totalImageSizeLBA = configuration.totalImageSizeLBA;
    if (totalImageSizeLBA > U32_MAX) {
        protectiveMBR.partitions[0].sizeLBA = U32_MAX;
    } else {
        protectiveMBR.partitions[0].sizeLBA = (U32)(totalImageSizeLBA - 1);
    }

    memcpy(fileBuffer, &protectiveMBR, sizeof(MBR));
}
