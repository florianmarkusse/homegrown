#include "image-builder/gpt.h"

typedef struct {
    U8 signature[8];
    U32 revision;
    U32 header_size;
    U32 header_crc32;
    U32 reserved_1;
    U64 my_lba;
    U64 alternate_lba;
    U64 first_usable_lba;
    U64 last_usable_lba;
    Guid disk_guid;
    U64 partition_table_lba;
    U32 number_of_entries;
    U32 size_of_entry;
    U32 partition_table_crc32;

    U8 reserved_2[512 - 92];
} __attribute__((packed)) Gpt_Header;

void writeGPT(WriteBuffer *file) {}
