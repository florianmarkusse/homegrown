#include "image-builder/gpt.h"
#include "image-builder/configuration.h"
#include "image-builder/crc32.h"
#include "platform-abstraction/memory/manipulation.h"
#include "shared/maths/maths.h"
#include "shared/uuid.h"
#include "uefi/guid.h"

#include <stdlib.h>

typedef struct {
    U8 signature[8];
    U32 revision;
    U32 headerSize;
    U32 headerCRC32;
    U32 reserved;
    U64 myLBA;
    U64 alternateLBA;
    U64 firstUsableLBA;
    U64 lastUsableLBA;
    UUID diskGUID;
    U64 partitionTableLBA;
    U32 numberOfEntries;
    U32 sizeOfEntry;
    U32 partitionTableCRC32;
    // The rest of the block is reserved by UEFI and must be zero. Length is
    // blockSize - sizeof(GPTHeader)
} __attribute__((packed)) GPTHeader;

typedef struct {
    UUID partitionTypeGUID;
    UUID uniquePartitionGUID;
    U64 startingLBA;
    U64 endingLBA;
    U64 attributes;
    U16 partitionName[36]; // UCS-2 (UTF-16 limited to code points 0x0000 -
                           // 0xFFFF)
} __attribute__((packed)) GPTPartitionEntry;

/*static UUID randomVersion4Variant2UUID() {*/
/*    UUID result;*/
/*    for (U8 i = 0; i < sizeof(UUID); i++) {*/
/*        result.u8[i] = rand() & 0xFF;*/
/*    }*/
/**/
/*    setUUIDType(&result, 4, UUID_VARIANT_2);*/
/**/
/*    return result;*/
/*}*/

// NOTE: Change this back maybe to actual randomness?
UUID RANDOM_GUID_1 = (UUID){.timeLo = 0x12345678,
                            .timeMid = 0xB9E5,
                            .timeHiAndVer = 0x4433,
                            .clockSeqHiAndRes = 0x87,
                            .clockSeqLo = 0xC0,
                            .node = {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};

UUID RANDOM_GUID_2 = (UUID){.timeLo = 0x87654321,
                            .timeMid = 0xB9E5,
                            .timeHiAndVer = 0x4433,
                            .clockSeqHiAndRes = 0x87,
                            .clockSeqLo = 0xC0,
                            .node = {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};
UUID RANDOM_GUID_3 = (UUID){.timeLo = 0x45612378,
                            .timeMid = 0xB9E5,
                            .timeHiAndVer = 0x4433,
                            .clockSeqHiAndRes = 0x87,
                            .clockSeqLo = 0xC0,
                            .node = {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};

static GPTHeader gptHeader = {
    .signature = {"EFI PART"},
    .revision = 0x00010000, // Version 1.0
    .headerSize = sizeof(GPTHeader),
    .headerCRC32 = 0, // NOTE: Calculated after filling out rest of structure
    .reserved = 0,
    .myLBA = SectionsInLBASize.PROTECTIVE_MBR,
    .alternateLBA = 0,   // NOTE Will calculate later
    .firstUsableLBA = 0, // NOTE Will calculate later
    .lastUsableLBA = 0,  // NOTE Will calculate later
    .diskGUID = {0},     // NOTE Will calculate later
    .partitionTableLBA = SectionsInLBASize.PROTECTIVE_MBR +
                         SectionsInLBASize.GPT_HEADER, // After MBR + GPT header
    .numberOfEntries = GPT_PARTITION_TABLE_ENTRIES,
    .sizeOfEntry = GPT_PARTITION_TABLE_ENTRY_SIZE,
    .partitionTableCRC32 = 0, // NOTE: Will calculate later
};

GPTPartitionEntry partitionEntries[GPT_PARTITION_TABLE_ENTRIES] = {
    {
        .partitionTypeGUID = EFI_SYSTEM_PARTITION_GUID,
        .uniquePartitionGUID = {0}, // NOTE Will calculate later
        .startingLBA = 0,           // NOTE Will calculate later
        .endingLBA = 0,             // NOTE Will calculate later
        .attributes = 0,
        .partitionName = u"EFI SYSTEM",
    },
    {
        .partitionTypeGUID = FLOS_BASIC_DATA_GUID,
        .uniquePartitionGUID = {0}, // NOTE Will calculate later
        .startingLBA = 0,           // NOTE Will calculate later
        .endingLBA = 0,             // NOTE Will calculate later
        .attributes = 0,
        .partitionName = u"BASIC DATA",
    },
};

void fillPartitionEntry(U32 index, U64 startLBA, U64 sizeLBA) {
    if (index == 0) {
        partitionEntries[index].uniquePartitionGUID = RANDOM_GUID_2;
    } else {
        partitionEntries[index].uniquePartitionGUID = RANDOM_GUID_3;
    }
    partitionEntries[index].startingLBA = startLBA;
    partitionEntries[index].endingLBA =
        partitionEntries[index].startingLBA + sizeLBA - 1;
}

void writeGPTs(U8 *fileBuffer) {
    gptHeader.alternateLBA =
        configuration.totalImageSizeLBA - SectionsInLBASize.GPT_HEADER;
    gptHeader.firstUsableLBA = SectionsInLBASize.PROTECTIVE_MBR +
                               SectionsInLBASize.GPT_HEADER +
                               configuration.GPTPartitionTableSizeLBA;
    gptHeader.lastUsableLBA = configuration.totalImageSizeLBA -
                              SectionsInLBASize.GPT_HEADER -
                              configuration.GPTPartitionTableSizeLBA - 1;
    gptHeader.diskGUID = RANDOM_GUID_1;

    fillPartitionEntry(0, configuration.EFISystemPartitionStartLBA,
                       configuration.EFISystemPartitionSizeLBA);
    fillPartitionEntry(1, configuration.dataPartitionStartLBA,
                       configuration.dataPartitionSizeLBA);

    gptHeader.partitionTableCRC32 =
        calculateCRC32(partitionEntries, sizeof(partitionEntries));
    gptHeader.headerCRC32 = calculateCRC32(&gptHeader, sizeof(GPTHeader));

    U8 *primaryBuffer = fileBuffer;
    primaryBuffer +=
        SectionsInLBASize.PROTECTIVE_MBR * configuration.LBASizeBytes;
    memcpy(primaryBuffer, &gptHeader, sizeof(GPTHeader));

    primaryBuffer += SectionsInLBASize.GPT_HEADER * configuration.LBASizeBytes;
    memcpy(primaryBuffer, partitionEntries, sizeof(partitionEntries));

    U64 primaryMyLBA = gptHeader.myLBA;
    gptHeader.myLBA = gptHeader.alternateLBA;
    gptHeader.alternateLBA = primaryMyLBA;

    gptHeader.partitionTableLBA = configuration.totalImageSizeLBA -
                                  SectionsInLBASize.GPT_HEADER -
                                  configuration.GPTPartitionTableSizeLBA;

    gptHeader.headerCRC32 = 0;
    gptHeader.headerCRC32 = calculateCRC32(&gptHeader, sizeof(GPTHeader));

    fileBuffer += configuration.totalImageSizeBytes;
    fileBuffer -= SectionsInLBASize.GPT_HEADER * configuration.LBASizeBytes;
    memcpy(fileBuffer, &gptHeader, sizeof(GPTHeader));

    fileBuffer -=
        configuration.GPTPartitionTableSizeLBA * configuration.LBASizeBytes;
    memcpy(fileBuffer, partitionEntries, sizeof(partitionEntries));
}
