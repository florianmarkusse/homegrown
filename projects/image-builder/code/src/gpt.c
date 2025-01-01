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

static UUID randomVersion4Variant2UUID() {
    UUID result;
    for (U8 i = 0; i < sizeof(UUID); i++) {
        result.u8[i] = rand() & 0xFF;
    }

    setUUIDType(&result, 4, UUID_VARIANT_2);

    return result;
}

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

void fillPartitionEntry(U32 index, U64 unalignedStartLBA, U64 sizeLBA) {
    partitionEntries[index].uniquePartitionGUID = randomVersion4Variant2UUID();
    partitionEntries[index].startingLBA =
        ALIGN_UP_VALUE(unalignedStartLBA, configuration.alignmentLBA);
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
    gptHeader.diskGUID = randomVersion4Variant2UUID();

    fillPartitionEntry(
        0, gptHeader.partitionTableLBA + configuration.GPTPartitionTableSizeLBA,
        configuration.EFISystemPartitionSizeLBA);
    fillPartitionEntry(1, partitionEntries[0].endingLBA + 1,
                       configuration.DataPartitionSizeLBA);

    gptHeader.partitionTableCRC32 =
        calculateCRC32(partitionEntries, sizeof(partitionEntries));
    gptHeader.headerCRC32 = calculateCRC32(&gptHeader, sizeof(GPTHeader));

    U8 *primaryBuffer = fileBuffer;
    primaryBuffer += SectionsInLBASize.PROTECTIVE_MBR * configuration.LBASize;
    memcpy(primaryBuffer, &gptHeader, sizeof(GPTHeader));

    primaryBuffer += SectionsInLBASize.GPT_HEADER * configuration.LBASize;
    memcpy(primaryBuffer, partitionEntries, sizeof(partitionEntries));

    gptHeader.headerCRC32 = 0;

    U64 primaryMyLBA = gptHeader.myLBA;
    gptHeader.myLBA = gptHeader.alternateLBA;
    gptHeader.alternateLBA = primaryMyLBA;

    gptHeader.partitionTableLBA = configuration.totalImageSizeLBA -
                                  SectionsInLBASize.GPT_HEADER -
                                  configuration.GPTPartitionTableSizeLBA - 1;

    gptHeader.headerCRC32 = calculateCRC32(&gptHeader, sizeof(GPTHeader));

    fileBuffer += configuration.totalImageSizeBytes;
    fileBuffer -= SectionsInLBASize.GPT_HEADER * configuration.LBASize;
    memcpy(fileBuffer, &gptHeader, sizeof(GPTHeader));

    fileBuffer -=
        configuration.GPTPartitionTableSizeLBA * configuration.LBASize;
    memcpy(fileBuffer, partitionEntries, sizeof(partitionEntries));
}
