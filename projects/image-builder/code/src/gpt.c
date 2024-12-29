#include "image-builder/gpt.h"
#include "image-builder/configuration.h"
#include "shared/uuid.h"
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

void writeGPT(WriteBuffer *file) {
    gptHeader.alternateLBA =
        configuration.totalImageSizeLBA - SectionsInLBASize.GPT_HEADER;
    gptHeader.firstUsableLBA = SectionsInLBASize.PROTECTIVE_MBR +
                               SectionsInLBASize.GPT_HEADER +
                               configuration.GPTPartitionTableSize;
    gptHeader.lastUsableLBA = configuration.totalImageSizeLBA -
                              SectionsInLBASize.GPT_HEADER -
                              configuration.GPTPartitionTableSize - 1;
    gptHeader.diskGUID = randomVersion4Variant2UUID();
}
