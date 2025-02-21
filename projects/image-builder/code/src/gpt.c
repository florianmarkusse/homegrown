#include "image-builder/gpt.h"

#include "abstraction/memory/manipulation.h"
#include "efi/uefi.h"
#include "image-builder/configuration.h"
#include "image-builder/crc32.h"
#include "shared/uuid.h"

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
    U16 partitionNameUTF16[36]; // UCS-2 (UTF-16 limited to code points 0x0000 -
                                // 0xFFFF)
} __attribute__((packed)) GPTPartitionEntry;

// My data partition GUID
// Windows/Linux have their own. We could use one of theirs but where's the fun
// in that?
static constexpr UUID FLOS_BASIC_DATA_GUID = {
    .timeLo = 0x5f68a13c,
    .timeMid = 0xcdae,
    .timeHiAndVer = 0x4372,
    .clockSeqHiAndRes = 0x95,
    .clockSeqLo = 0xc7,
    .node = {0xfb, 0xc3, 0x8a, 0x42, 0xff, 0x3e}};

static constexpr UUID RANDOM_GUID_1 =
    (UUID){.timeLo = 0x314D4330,
           .timeMid = 0x29de,
           .timeHiAndVer = 0x4029,
           .clockSeqHiAndRes = 0xA6,
           .clockSeqLo = 0x0D,
           .node = {0x89, 0xEA, 0x41, 0x02, 0x6A, 0x5D}};
static constexpr UUID RANDOM_GUID_2 =
    (UUID){.timeLo = 0x1E736CF5,
           .timeMid = 0x0F0F,
           .timeHiAndVer = 0x47B4,
           .clockSeqHiAndRes = 0x8E,
           .clockSeqLo = 0xE0,
           .node = {0xB6, 0xB5, 0x01, 0x0C, 0x05, 0xDD}};
static constexpr UUID RANDOM_GUID_3 =
    (UUID){.timeLo = 0x1B67CFAB,
           .timeMid = 0x21EF,
           .timeHiAndVer = 0x4033,
           .clockSeqHiAndRes = 0xB9,
           .clockSeqLo = 0xDD,
           .node = {0xCE, 0x61, 0x8C, 0xB3, 0x2C, 0xB8}};

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
        .partitionNameUTF16 = u"EFI SYSTEM",
    },
    {
        .partitionTypeGUID = FLOS_BASIC_DATA_GUID,
        .uniquePartitionGUID = {0}, // NOTE Will calculate later
        .startingLBA = 0,           // NOTE Will calculate later
        .endingLBA = 0,             // NOTE Will calculate later
        .attributes = 0,
        .partitionNameUTF16 = u"BASIC DATA",
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
    gptHeader.headerCRC32 = calculateCRC32(&gptHeader, gptHeader.headerSize);

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

    fileBuffer += gptHeader.partitionTableLBA * configuration.LBASizeBytes;
    memcpy(fileBuffer, partitionEntries, sizeof(partitionEntries));

    fileBuffer += (gptHeader.myLBA - gptHeader.partitionTableLBA) *
                  configuration.LBASizeBytes;
    memcpy(fileBuffer, &gptHeader, sizeof(GPTHeader));
}
