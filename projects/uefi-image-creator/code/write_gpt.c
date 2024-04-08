#include "crc32-table.h"
#include "util/log.h"
#include <errno.h>    // for errno
#include <inttypes.h> // for uint32_t, uint8_t, uint16_t, uint64_t, int32_t
#include <stdbool.h>  // for true, false, bool
#include <stdio.h>    // for fwrite, fprintf, fseek, stderr, fclose, fopen
#include <stdlib.h>   // for free, calloc, malloc, strtol, EXIT_FAILURE, rand
#include <string.h>   // for strcmp, memcpy, strlen, strcpy, strncat, strncpy
#include <sys/mman.h> // for mmap, munmap, MAP_ANONYMOUS, MAP_...
#include <time.h>     // for tm, time, localtime, time_t
#include <uchar.h>    // for char16_t

size_t memoryCap = (size_t)1 << 21;
// Note: we open files in this program which need to be closed when a program
// exits on error.
void *fileCloser[5];
typedef FLO_MAX_LENGTH_ARRAY(FILE *) flo_FILEPtr_max_a;
#define MAX_OPEN_FILES 64
// TODO: check returns of fseek beby
// TODO: add all files when open to this stack/array
FILE *openFilesBuf[MAX_OPEN_FILES];
flo_FILEPtr_max_a openedFiles = {
    .buf = openFilesBuf, .cap = MAX_OPEN_FILES, .len = 0};

void *memoryErrors[5];

void checkedFwrite(void *data, uint64_t size, FILE *fd) {
    if (fwrite(data, 1, size, fd) != size) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(FLO_STRING("Could not write to file descriptor!\n"));
        }
        FLO_ASSERT(false);
        __builtin_longjmp(fileCloser, 1);
    }
}

// -------------------------------------
// Global Typedefs
// -------------------------------------
// Globally Unique IDentifier (aka UUID)
typedef struct {
    uint32_t time_lo;
    uint16_t time_mid;
    uint16_t time_hi_and_ver;     // Highest 4 bits are version #
    uint8_t clock_seq_hi_and_res; // Highest bits are variant #
    uint8_t clock_seq_lo;
    uint8_t node[6];
} __attribute__((packed)) Guid;

// MBR Partition
typedef struct {
    uint8_t boot_indicator;
    uint8_t starting_chs[3];
    uint8_t os_type;
    uint8_t ending_chs[3];
    uint32_t starting_lba;
    uint32_t size_lba;
} __attribute__((packed)) Mbr_Partition;

// Master Boot Record
typedef struct {
    uint8_t boot_code[440];
    uint32_t mbr_signature;
    uint16_t unknown;
    Mbr_Partition partition[4];
    uint16_t boot_signature;
} __attribute__((packed)) Mbr;

// GPT Header
typedef struct {
    uint8_t signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved_1;
    uint64_t my_lba;
    uint64_t alternate_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    Guid disk_guid;
    uint64_t partition_table_lba;
    uint32_t number_of_entries;
    uint32_t size_of_entry;
    uint32_t partition_table_crc32;

    uint8_t reserved_2[512 - 92];
} __attribute__((packed)) Gpt_Header;

// GPT Partition Entry
typedef struct {
    Guid partition_type_guid;
    Guid unique_guid;
    uint64_t starting_lba;
    uint64_t ending_lba;
    uint64_t attributes;
    char16_t name[36]; // UCS-2 (UTF-16 limited to code points 0x0000 - 0xFFFF)
} __attribute__((packed)) Gpt_Partition_Entry;

// FAT32 Volume Boot Record (VBR)
typedef struct {
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytesPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint8_t BS_VolID[4];
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];

    // Not in fatgen103.doc tables
    uint8_t boot_code[510 - 90];
    uint16_t bootsect_sig; // 0xAA55
} __attribute__((packed)) Vbr;

// FAT32 File System Info Sector
typedef struct {
    uint32_t FSI_LeadSig;
    uint8_t FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    uint8_t FSI_Reserved2[12];
    uint32_t FSI_TrailSig;
} __attribute__((packed)) FSInfo;

// FAT32 Directory Entry (Short Name)
typedef struct {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__((packed)) FAT32_Dir_Entry_Short;

// FAT32 Directory Entry Attributes
typedef enum {
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN = 0x02,
    ATTR_SYSTEM = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE = 0x20,
    ATTR_LONG_NAME =
        ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID,
} FAT32_Dir_Attr;

// FAT32 File "types"
typedef enum {
    TYPE_DIR,  // Directory
    TYPE_FILE, // Regular file
} File_Type;

#define MAX_FILES 10
#define MAX_FILE_LEN (1 << 8)

// Internal Options object for commandline args
typedef struct {
    char *image_name;
    uint16_t lba_size;
    uint32_t esp_size;
    uint32_t data_size;
    char esp_file_paths[MAX_FILES][MAX_FILE_LEN];
    uint32_t num_esp_file_paths;
    FILE *esp_files[MAX_FILES];
    // TODO: why do esp files work with a file pointer and this exits later?
    char data_files[MAX_FILES][MAX_FILE_LEN];
    uint32_t num_data_files;
} Options;

static Options options = {.image_name = "test.hdd",
                          .lba_size = 512,
                          .esp_size = 1024 * 1024 * 33,
                          .data_size = 1024 * 1024 * 1};

// -------------------------------------
// Global constants, enums
// -------------------------------------
// EFI System Partition GUID
Guid ESP_GUID = {0xC12A7328, 0xF81F, 0x11D2,
                 0xBA,       0x4B,   {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B}};

// (Microsoft) Basic Data GUID
Guid BASIC_DATA_GUID = {0xEBD0A0A2, 0xB9E5,
                        0x4433,     0x87,
                        0xC0,       {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7}};

enum {
    GPT_TABLE_ENTRY_SIZE = 128,
    NUMBER_OF_GPT_TABLE_ENTRIES = 128,
    GPT_TABLE_SIZE = 16384, // Minimum size per UEFI spec 2.10
    ALIGNMENT = 1048576,    // 1 MiB alignment value
};

// -------------------------------------
// Global Variables
// -------------------------------------
uint32_t image_size = 0;
uint32_t esp_size_lbas = 0, data_size_lbas = 0, image_size_lbas = 0,
         gpt_table_lbas = 0; // Sizes in lbas
uint32_t align_lba = 0, esp_lba = 0, data_lba = 0, fat32_fats_lba = 0,
         fat32_data_lba = 0; // Starting LBA values

uint64_t bytes_to_lbas(const uint64_t bytes) {
    return (bytes + (options.lba_size - 1)) / options.lba_size;
}

// =====================================
// Pad out 0s to full lba size
// =====================================
void write_full_options(FILE *image) {
    uint8_t zero_sector[512];
    for (uint8_t i = 0;
         i < (options.lba_size - sizeof zero_sector) / sizeof zero_sector;
         i++) {
        checkedFwrite(zero_sector, sizeof zero_sector, image);
    }
}

uint64_t next_aligned_lba(uint64_t lba) {
    return (((lba + (align_lba - 1)) / align_lba) + 1) * align_lba;
}

// =====================================
// Create a new Version 4 Variant 2 GUID
// =====================================
Guid new_guid(void) {
    uint8_t rand_arr[16] = {0};

    for (uint8_t i = 0; i < sizeof rand_arr; i++) {
        rand_arr[i] = rand() & 0xFF; // Equivalent to modulo 256
    }

    // Fill out GUID
    Guid result = {
        .time_lo = *(uint32_t *)&rand_arr[0],
        .time_mid = *(uint16_t *)&rand_arr[4],
        .time_hi_and_ver = *(uint16_t *)&rand_arr[6],
        .clock_seq_hi_and_res = rand_arr[8],
        .clock_seq_lo = rand_arr[9],
        .node = {rand_arr[10], rand_arr[11], rand_arr[12], rand_arr[13],
                 rand_arr[14], rand_arr[15]},
    };

    // Fill out version bits - version 4
    result.time_hi_and_ver &= ~(1 << 15); // 0b_0_111 1111
    result.time_hi_and_ver |= (1 << 14);  // 0b0_1_00 0000
    result.time_hi_and_ver &= ~(1 << 13); // 0b11_0_1 1111
    result.time_hi_and_ver &= ~(1 << 12); // 0b111_0_ 1111

    // Fill out variant bits
    result.clock_seq_hi_and_res |= (1 << 7);  // 0b_1_000 0000
    result.clock_seq_hi_and_res |= (1 << 6);  // 0b0_1_00 0000
    result.clock_seq_hi_and_res &= ~(1 << 5); // 0b11_0_1 1111

    return result;
}

// =====================================
// Get new date/time values for FAT32 directory entries
// =====================================
void get_fat_dir_entry_time_date(uint16_t *in_time, uint16_t *in_date) {
    time_t curr_time;
    curr_time = time(NULL);
    struct tm tm = *localtime(&curr_time);

    // FAT32 needs # of years since 1980, localtime returns tm_year as # years
    // since 1900,
    //   subtract 80 years for correct year value. Also convert month of year
    //   from 0-11 to 1-12 by adding 1
    *in_date = (uint16_t)(((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) |
                          tm.tm_mday);

    // Seconds is # 2-second count, 0-29
    if (tm.tm_sec == 60)
        tm.tm_sec = 59;
    *in_time = (uint16_t)(tm.tm_hour << 11 | tm.tm_min << 5 | (tm.tm_sec / 2));
}

// =====================================
// Write protective MBR
// =====================================
void write_mbr(FILE *image) {
    uint64_t mbr_image_lbas = image_size_lbas;
    if (mbr_image_lbas > 0xFFFFFFFF) {
        mbr_image_lbas = 0x100000000;
    }

    Mbr mbr = {
        .boot_code = {0},
        .mbr_signature = 0,
        .unknown = 0,
        .partition[0] =
            {
                .boot_indicator = 0,
                .starting_chs = {0x00, 0x02, 0x00},
                .os_type = 0xEE, // Protective GPT
                .ending_chs = {0xFF, 0xFF, 0xFF},
                .starting_lba = 0x00000001,
                .size_lba = (uint32_t)(mbr_image_lbas - 1),
            },
        .boot_signature = 0xAA55,
    };

    checkedFwrite(&mbr, sizeof mbr, image);

    write_full_options(image);
}
// =====================================
// Write GPT headers & tables, primary & secondary
// =====================================
void write_gpts(FILE *image) {
    // Fill out primary GPT header
    Gpt_Header primary_gpt = {
        .signature = {"EFI PART"},
        .revision = 0x00010000, // Version 1.0
        .header_size = 92,
        .header_crc32 = 0, // Will calculate later
        .reserved_1 = 0,
        .my_lba = 1, // LBA 1 is right after MBR
        .alternate_lba = image_size_lbas - 1,
        .first_usable_lba =
            1 + 1 + gpt_table_lbas, // MBR + GPT header + primary gpt table
        .last_usable_lba =
            image_size_lbas - 1 - gpt_table_lbas - 1, // 2nd GPT header + table
        .disk_guid = new_guid(),
        .partition_table_lba = 2, // After MBR + GPT header
        .number_of_entries = 128,
        .size_of_entry = 128,
        .partition_table_crc32 = 0, // Will calculate later
        .reserved_2 = {0},
    };

    // Fill out primary table partition entries
    Gpt_Partition_Entry gpt_table[NUMBER_OF_GPT_TABLE_ENTRIES] = {
        // EFI System Paritition
        {
            .partition_type_guid = ESP_GUID,
            .unique_guid = new_guid(),
            .starting_lba = esp_lba,
            .ending_lba = esp_lba + esp_size_lbas,
            .attributes = 0,
            .name = u"EFI SYSTEM",
        },

        // Basic Data Paritition
        {
            .partition_type_guid = BASIC_DATA_GUID,
            .unique_guid = new_guid(),
            .starting_lba = data_lba,
            .ending_lba = data_lba + data_size_lbas,
            .attributes = 0,
            .name = u"BASIC DATA",
        },
    };

    primary_gpt.partition_table_crc32 =
        calculateCRC32(gpt_table, sizeof gpt_table);
    primary_gpt.header_crc32 =
        calculateCRC32(&primary_gpt, primary_gpt.header_size);

    checkedFwrite(&primary_gpt, sizeof primary_gpt, image);
    write_full_options(image);

    checkedFwrite(&gpt_table, sizeof gpt_table, image);

    Gpt_Header secondary_gpt = primary_gpt;

    secondary_gpt.header_crc32 = 0;
    secondary_gpt.partition_table_crc32 = 0;
    secondary_gpt.my_lba = primary_gpt.alternate_lba;
    secondary_gpt.alternate_lba = primary_gpt.my_lba;
    secondary_gpt.partition_table_lba = image_size_lbas - 1 - gpt_table_lbas;

    secondary_gpt.partition_table_crc32 =
        calculateCRC32(gpt_table, sizeof gpt_table);
    secondary_gpt.header_crc32 =
        calculateCRC32(&secondary_gpt, secondary_gpt.header_size);

    fseek(image, secondary_gpt.partition_table_lba * options.lba_size,
          SEEK_SET);

    checkedFwrite(&gpt_table, sizeof gpt_table, image);

    checkedFwrite(&secondary_gpt, sizeof secondary_gpt, image);
    write_full_options(image);
}

// =====================================
// Write EFI System Partition (ESP) w/FAT32 filesystem
// =====================================
void write_esp(FILE *image) {
    // Reserved sectors region --------------------------
    // Fill out Volume Boot Record (VBR)
    uint8_t reserved_sectors = 32; // FAT32
    Vbr vbr = {
        .BS_jmpBoot = {0xEB, 0x00, 0x90},
        .BS_OEMName = {"THISDISK"},
        .BPB_BytesPerSec = options.lba_size,
        .BPB_SecPerClus = 1,
        .BPB_RsvdSecCnt = reserved_sectors,
        .BPB_NumFATs = 2,
        .BPB_RootEntCnt = 0,
        .BPB_TotSec16 = 0,
        .BPB_Media = 0xF8, // "Fixed" non-removable media; Could also be 0xF0
                           // for e.g. flash drive
        .BPB_FATSz16 = 0,
        .BPB_SecPerTrk = 0,
        .BPB_NumHeads = 0,
        .BPB_HiddSec = esp_lba - 1, // # of sectors before this partition/volume
        .BPB_TotSec32 = esp_size_lbas, // Size of this partition
        .BPB_FATSz32 = (align_lba - reserved_sectors) /
                       2,  // Align data region on alignment value
        .BPB_ExtFlags = 0, // Mirrored FATs
        .BPB_FSVer = 0,
        .BPB_RootClus =
            2, // Clusters 0 & 1 are reserved; root dir cluster starts at 2
        .BPB_FSInfo = 1, // Sector 0 = this VBR; FS Info sector follows it
        .BPB_BkBootSec = 6,
        .BPB_Reserved = {0},
        .BS_DrvNum = 0x80, // 1st hard drive
        .BS_Reserved1 = 0,
        .BS_BootSig = 0x29,
        .BS_VolID = {0},
        .BS_VolLab = {"NO NAME    "}, // No volume label
        .BS_FilSysType = {"FAT32   "},

        // Not in fatgen103.doc tables
        .boot_code = {0},
        .bootsect_sig = 0xAA55,
    };

    // Fill out file system info sector
    FSInfo fsinfo = {
        .FSI_LeadSig = 0x41615252,
        .FSI_Reserved1 = {0},
        .FSI_StrucSig = 0x61417272,
        .FSI_Free_Count = 0xFFFFFFFF,
        .FSI_Nxt_Free =
            5, // First available cluster (value = 0) after /EFI/BOOT
        .FSI_Reserved2 = {0},
        .FSI_TrailSig = 0xAA550000,
    };

    fat32_fats_lba = esp_lba + vbr.BPB_RsvdSecCnt;
    fat32_data_lba = fat32_fats_lba + (vbr.BPB_NumFATs * vbr.BPB_FATSz32);

    // Write VBR and FSInfo sector
    fseek(image, esp_lba * options.lba_size, SEEK_SET);

    checkedFwrite(&vbr, sizeof vbr, image);
    write_full_options(image);

    checkedFwrite(&fsinfo, sizeof fsinfo, image);
    write_full_options(image);

    // Go to backup boot sector location
    fseek(image, (esp_lba + vbr.BPB_BkBootSec) * options.lba_size, SEEK_SET);

    // Write VBR and FSInfo at backup location
    fseek(image, esp_lba * options.lba_size, SEEK_SET);

    checkedFwrite(&vbr, sizeof vbr, image);
    write_full_options(image);

    checkedFwrite(&fsinfo, sizeof fsinfo, image);
    write_full_options(image);

    // FAT region --------------------------
    // Write FATs (NOTE: FATs will be mirrored)
    for (uint8_t i = 0; i < vbr.BPB_NumFATs; i++) {
        fseek(image,
              (fat32_fats_lba + (i * vbr.BPB_FATSz32)) * options.lba_size,
              SEEK_SET);

        uint32_t cluster = 0;

        // Cluster 0; FAT identifier, lowest 8 bits are the media type/byte
        cluster = 0xFFFFFF00 | vbr.BPB_Media;
        checkedFwrite(&cluster, sizeof cluster, image);

        // Cluster 1; End of Chain (EOC) marker
        cluster = 0xFFFFFFFF;
        checkedFwrite(&cluster, sizeof cluster, image);

        // Cluster 2; Root dir '/' cluster start, if end of file/dir data then
        // write EOC marker
        cluster = 0xFFFFFFFF;
        checkedFwrite(&cluster, sizeof cluster, image);

        // Cluster 3; '/EFI' dir cluster
        cluster = 0xFFFFFFFF;
        checkedFwrite(&cluster, sizeof cluster, image);

        // Cluster 4; '/EFI/BOOT' dir cluster
        cluster = 0xFFFFFFFF;
        checkedFwrite(&cluster, sizeof cluster, image);

        // Cluster 5+; Other files/directories...
        // e.g. if adding a file with a size = 5 sectors/clusters
        // cluster = 6;    // Point to next cluster containing file data
        // cluster = 7;    // Point to next cluster containing file data
        // cluster = 8;    // Point to next cluster containing file data
        // cluster = 9;    // Point to next cluster containing file data
        // cluster = 0xFFFFFFFF; // EOC marker, no more file data after this
        // cluster
    }

    // Data region --------------------------
    // Write File/Dir data...
    fseek(image, fat32_data_lba * options.lba_size, SEEK_SET);

    // Root '/' Directory entries
    // "/EFI" dir entry
    FAT32_Dir_Entry_Short dir_ent = {
        .DIR_Name = {"EFI        "},
        .DIR_Attr = ATTR_DIRECTORY,
        .DIR_NTRes = 0,
        .DIR_CrtTimeTenth = 0,
        .DIR_CrtTime = 0,
        .DIR_CrtDate = 0,
        .DIR_LstAccDate = 0,
        .DIR_FstClusHI = 0,
        .DIR_WrtTime = 0,
        .DIR_WrtDate = 0,
        .DIR_FstClusLO = 3,
        .DIR_FileSize = 0, // Directories have 0 file size
    };

    uint16_t create_time = 0, create_date = 0;
    get_fat_dir_entry_time_date(&create_time, &create_date);

    dir_ent.DIR_CrtTime = create_time;
    dir_ent.DIR_CrtDate = create_date;
    dir_ent.DIR_WrtTime = create_time;
    dir_ent.DIR_WrtDate = create_date;

    checkedFwrite(&dir_ent, sizeof dir_ent, image);

    // /EFI Directory entries
    fseek(image, (fat32_data_lba + 1) * options.lba_size, SEEK_SET);

    memcpy(dir_ent.DIR_Name, ".          ",
           11); // "." dir entry, this directory itself
    checkedFwrite(&dir_ent, sizeof dir_ent, image);

    memcpy(dir_ent.DIR_Name, "..         ",
           11);                // ".." dir entry, parent dir (ROOT dir)
    dir_ent.DIR_FstClusLO = 0; // Root directory does not have a cluster value
    checkedFwrite(&dir_ent, sizeof dir_ent, image);

    memcpy(dir_ent.DIR_Name, "BOOT       ", 11); // /EFI/BOOT directory
    dir_ent.DIR_FstClusLO = 4;                   // /EFI/BOOT cluster
    checkedFwrite(&dir_ent, sizeof dir_ent, image);

    // /EFI/BOOT Directory entries
    fseek(image, (fat32_data_lba + 2) * options.lba_size, SEEK_SET);

    memcpy(dir_ent.DIR_Name, ".          ",
           11); // "." dir entry, this directory itself
    checkedFwrite(&dir_ent, sizeof dir_ent, image);

    memcpy(dir_ent.DIR_Name, "..         ",
           11);                // ".." dir entry, parent dir (/EFI dir)
    dir_ent.DIR_FstClusLO = 3; // /EFI directory cluster
    checkedFwrite(&dir_ent, sizeof dir_ent, image);
}

// =============================
// Add a new directory or file to a given parent directory
// =============================
bool add_file_to_esp(char *file_name, FILE *file, FILE *image, File_Type type,
                     uint32_t *parent_dir_cluster) {
    // First grab FAT32 filesystem info for VBR and File System info
    Vbr vbr = {0};
    fseek(image, esp_lba * options.lba_size, SEEK_SET);
    if (fread(&vbr, 1, sizeof vbr, image) != sizeof vbr) {
        fprintf(stderr, "Error: Could not fead vbr.\n");
        return false;
    }

    FSInfo fsinfo = {0};
    fseek(image, (esp_lba + 1) * options.lba_size, SEEK_SET);
    if (fread(&fsinfo, 1, sizeof fsinfo, image) != sizeof fsinfo) {
        fprintf(stderr, "Error: Could not fead fsinfo.\n");
        return false;
    }

    // Get file size of file
    uint64_t file_size_bytes = 0, file_size_lbas = 0;
    if (type == TYPE_FILE) {
        fseek(file, 0, SEEK_END);
        file_size_bytes = ftell(file);
        file_size_lbas = bytes_to_lbas(file_size_bytes);
        rewind(file);
    }

    // Get next free cluster in FATs
    uint32_t next_free_cluster = fsinfo.FSI_Nxt_Free;
    uint32_t starting_cluster =
        next_free_cluster; // Starting cluster for new dir/file

    // Add new clusters to FATs
    for (uint8_t i = 0; i < vbr.BPB_NumFATs; i++) {
        fseek(image,
              (fat32_fats_lba + (i * vbr.BPB_FATSz32)) * options.lba_size,
              SEEK_SET);
        fseek(image, next_free_cluster * sizeof next_free_cluster, SEEK_CUR);

        uint32_t cluster = fsinfo.FSI_Nxt_Free;
        next_free_cluster = cluster;
        if (type == TYPE_FILE) {
            for (uint64_t lba = 0; lba < file_size_lbas - 1; lba++) {
                cluster++; // Each cluster points to next cluster of file data
                next_free_cluster++;
                checkedFwrite(&cluster, sizeof cluster, image);
            }
        }

        // Write EOC marker cluster, this would be the only cluster added for a
        // directory
        //   (type == TYPE_DIR)
        cluster = 0xFFFFFFFF;
        next_free_cluster++;
        checkedFwrite(&cluster, sizeof cluster, image);
    }

    // Update next free cluster in FS Info
    fsinfo.FSI_Nxt_Free = next_free_cluster;
    fseek(image, (esp_lba + 1) * options.lba_size, SEEK_SET);
    checkedFwrite(&fsinfo, sizeof fsinfo, image);

    // Go to Parent Directory's data location in data region
    fseek(image, (fat32_data_lba + *parent_dir_cluster - 2) * options.lba_size,
          SEEK_SET);

    // Add new directory entry for this new dir/file at end of current
    // dir_entrys
    FAT32_Dir_Entry_Short dir_entry = {0};

    while (fread(&dir_entry, 1, sizeof dir_entry, image) == sizeof dir_entry &&
           dir_entry.DIR_Name[0] != '\0') {
        ;
    }

    // sizeof dir_entry = 32, back up to overwrite this empty spot
    fseek(image, -32, SEEK_CUR);

    // Check name length for FAT 8.3 naming
    char *dot_pos = strchr(file_name, '.');
    uint64_t name_len = strlen(file_name);
    if ((!dot_pos && name_len > 11) || (dot_pos && name_len > 12) ||
        (dot_pos && dot_pos - file_name > 8)) {
        return false; // Name is too long or invalid
    }

    // Convert name to FAT 8.3 naming
    // e.g. "FOO.BAR"  -> "FOO     BAR",
    //      "BA.Z"     -> "BA      Z  ",
    //      "ELEPHANT" -> "ELEPHANT   "
    memset(dir_entry.DIR_Name, ' ',
           11); // Start with all spaces, name/ext will be space padded

    if (dot_pos) {
        uint8_t i = 0;
        // Name 8 portion of 8.3
        for (i = 0; i < (dot_pos - file_name); i++)
            dir_entry.DIR_Name[i] = file_name[i];

        uint8_t j = i;
        while (i < 8)
            dir_entry.DIR_Name[i++] = ' ';

        if (file_name[j] == '.')
            j++; // Skip dot to get to extension

        // Extension 3 portion of 8.3
        while (file_name[j])
            dir_entry.DIR_Name[i++] = file_name[j++];

        while (i < 11)
            dir_entry.DIR_Name[i++] = ' ';
    } else {
        memcpy(dir_entry.DIR_Name, file_name, name_len);
    }

    if (type == TYPE_DIR) {
        dir_entry.DIR_Attr = ATTR_DIRECTORY;
    }

    uint16_t fat_time, fat_date;
    get_fat_dir_entry_time_date(&fat_time, &fat_date);
    dir_entry.DIR_CrtTime = fat_time;
    dir_entry.DIR_CrtDate = fat_date;
    dir_entry.DIR_WrtTime = fat_time;
    dir_entry.DIR_WrtDate = fat_date;

    dir_entry.DIR_FstClusHI = (starting_cluster >> 16) & 0xFFFF;
    dir_entry.DIR_FstClusLO = starting_cluster & 0xFFFF;

    if (type == TYPE_FILE) {
        dir_entry.DIR_FileSize = (uint32_t)file_size_bytes;
    }

    checkedFwrite(&dir_entry, sizeof dir_entry, image);

    // Go to this new file's cluster's data location in data region
    fseek(image, (fat32_data_lba + starting_cluster - 2) * options.lba_size,
          SEEK_SET);

    // Add new file data
    // For directory add dir_entrys for "." and ".."
    if (type == TYPE_DIR) {
        memcpy(dir_entry.DIR_Name, ".          ",
               11); // "." dir_entry; this directory itself
        checkedFwrite(&dir_entry, sizeof dir_entry, image);

        memcpy(dir_entry.DIR_Name, "..         ",
               11); // ".." dir_entry; parent directory
        dir_entry.DIR_FstClusHI = (*parent_dir_cluster >> 16) & 0xFFFF;
        dir_entry.DIR_FstClusLO = *parent_dir_cluster & 0xFFFF;
        checkedFwrite(&dir_entry, sizeof dir_entry, image);
    } else {
        // For file, add file data
        uint8_t *file_buf = calloc(1, options.lba_size);
        for (uint64_t i = 0; i < file_size_lbas; i++) {
            // In case last lba is less than a full lba in size, use actual
            // bytes read
            //   to write file to disk image
            size_t bytes_read = fread(file_buf, 1, options.lba_size, file);
            checkedFwrite(file_buf, bytes_read, image);
        }
    }

    // Set dir_cluster for new parent dir, if a directory was just added
    if (type == TYPE_DIR) {
        *parent_dir_cluster = starting_cluster;
    }

    return true;
}

// =============================
// Add a file path to the EFI System Partition;
//   will add new directories if not found, and
//   new file at end of path
// =============================
bool add_path_to_esp(char *path, FILE *file, FILE *image) {
    // Parse input path for each name
    if (*path != '/') {
        return false; // Path must begin with root '/'
    }

    File_Type type = TYPE_DIR;
    char *start = path + 1; // Skip initial slash
    char *end = start;
    uint32_t dir_cluster =
        2; // Next directory's cluster location; start at root

    // Get next name from path, until reached end of path for file to add
    while (type == TYPE_DIR) {
        while (*end != '/' && *end != '\0') {
            end++;
        }

        if (*end == '/') {
            type = TYPE_DIR;
        } else {
            type = TYPE_FILE; // Reached end of path
        }

        *end = '\0'; // Null terminate next name in case of directory

        // Search for name in current directory's file data (dir_entrys)
        FAT32_Dir_Entry_Short dir_entry = {0};
        bool found = false;
        fseek(image, (fat32_data_lba + dir_cluster - 2) * options.lba_size,
              SEEK_SET);
        do {
            if (fread(&dir_entry, 1, sizeof dir_entry, image) ==
                    sizeof dir_entry &&
                !memcmp(dir_entry.DIR_Name, start, strlen(start))) {
                // Found name in directory, save cluster for last directory
                // found
                dir_cluster =
                    (dir_entry.DIR_FstClusHI << 16) | dir_entry.DIR_FstClusLO;
                found = true;
                break;
            }
        } while (dir_entry.DIR_Name[0] != '\0');

        if (!found) {
            // Add new directory or file to last found directory;
            //   if new directory, update current directory cluster to check/use
            //   for next new files
            if (!add_file_to_esp(start, file, image, type, &dir_cluster)) {
                return false;
            }
        }

        *end++ = '/';
        start = end;
    }

    *--end = '\0'; // Don't add extra slash to end of path, final file name is
                   // not a directory

    // Show info to user
    printf("Added '%s' to EFI System Partition\n", path);

    return true;
}

// =============================
// Add disk image info file to hold at minimum the size of this disk image
// =============================
bool add_disk_image_info_file(FILE *image) {
    char *file_buf = calloc(1, options.lba_size);
    snprintf(file_buf, options.lba_size, "DISK_SIZE=%u\n", image_size);

    FILE *fp = fopen("DSKIMG.INF", "wbe");
    if (!fp) {
        return false;
    }

    checkedFwrite(file_buf, strlen(file_buf), fp);
    fclose(fp);
    fp = fopen("DSKIMG.INF", "rbe");

    char path[25] = {0};
    strcpy(path, "/EFI/BOOT/DSKIMG.INF");
    if (!add_path_to_esp(path, fp, image)) {
        return false;
    }
    fclose(fp);

    return true;
}

// =============================
// Add file to the Basic Data Partition
// =============================
bool add_file_to_data_partition(char *filepath, FILE *image) {
    // Will save location of next spot to put a file in
    static uint64_t starting_lba = 0;

    // Go to data partition
    fseek(image, (data_lba + starting_lba) * options.lba_size, SEEK_SET);

    FILE *fp = fopen(filepath, "rbe");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filepath);
        return false;
    }

    // Get file size
    uint64_t file_size_bytes = 0, file_size_lbas = 0;
    fseek(fp, 0, SEEK_END);
    file_size_bytes = ftell(fp);
    file_size_lbas = bytes_to_lbas(file_size_bytes);
    rewind(fp);

    // Check if adding next file will overrun data partition size
    if ((starting_lba + file_size_lbas) * options.lba_size >=
        options.data_size) {
        fprintf(stderr,
                "Error: Can't add file %s to Data Partition; "
                "Data Partition size is %u (%u"
                " LBAs) and all files added "
                "would overrun this size\n",
                filepath, options.data_size, data_size_lbas);
    }

    uint8_t *file_buf = calloc(1, options.lba_size);
    for (uint64_t i = 0; i < file_size_lbas; i++) {
        uint64_t bytes_read = fread(file_buf, 1, options.lba_size, fp);
        checkedFwrite(file_buf, bytes_read, image);
    }
    fclose(fp);

    // Print info to user
    char *name = NULL;
    char *slash = strrchr(filepath, '/');
    if (!slash) {
        name = filepath;
    } else {
        name = slash + 1;
    }

    printf("Added '%s' from path '%s' to Data Partition\n", name, filepath);

    // Add to info file for each file added
    static bool first_file = true;
    char info_file[12] = "DATAFLS.INF"; // "Data (partition) files info"

    if (first_file) {
        first_file = false;
        fp = fopen(info_file, "wbe"); // Truncate before writing
    } else {
        fp = fopen(info_file, "abe"); // Add to end of previous info
    }

    if (!fp) {
        fprintf(stderr, "Error: Could not open file '%s'\n", info_file);
        return false;
    }

    file_buf = calloc(1, options.lba_size);
    snprintf((char *)file_buf, options.lba_size,
             "FILE_NAME=%s\t"
             "FILE_SIZE=%" PRIu64 "\t"
             "DISK_LBA=%" PRIu64 "\n", // Add newline between files
             name, file_size_bytes,
             data_lba + starting_lba); // Offset from start of data partition

    checkedFwrite(file_buf, strlen((char *)file_buf), fp);
    fclose(fp);

    // Set next spot to write a file at
    starting_lba += file_size_lbas;

    return true;
}

void writeUEFIImage(flo_arena scratch) {
    FILE *image = NULL, *fp = NULL;

    // Set sizes & LBA values
    gpt_table_lbas = GPT_TABLE_SIZE / options.lba_size;

    // Add extra padding for:
    //   2 aligned partitions
    //   2 GPT tables
    //   MBR
    //   GPT headers
    uint32_t padding =
        (ALIGNMENT * 2 + (options.lba_size * ((gpt_table_lbas * 2) + 1 + 2)));
    image_size = options.esp_size + options.data_size + padding;
    image_size_lbas = (uint32_t)bytes_to_lbas(image_size);
    align_lba = ALIGNMENT / options.lba_size;
    esp_lba = align_lba;
    esp_size_lbas = (uint32_t)bytes_to_lbas(options.esp_size);
    data_size_lbas = (uint32_t)bytes_to_lbas(options.data_size);
    data_lba = (uint32_t)next_aligned_lba(esp_lba + esp_size_lbas);

    // Open image file
    image = fopen(options.image_name, "wb+e");
    if (!image) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(FLO_STRING("Error: could not open file "));
            FLO_ERROR(options.image_name, FLO_NEWLINE);
        }
        __builtin_longjmp(fileCloser, 1);
    }

    // Print info on sizes and image for user
    FLO_FLUSH_AFTER(FLO_STDOUT) {
        FLO_INFO("IMAGE NAME: ");
        FLO_INFO(options.image_name, FLO_NEWLINE);

        FLO_INFO("LBA SIZE: ");
        FLO_INFO(options.lba_size, FLO_NEWLINE);

        FLO_INFO("ESP SIZE: ");
        FLO_INFO(options.esp_size / ALIGNMENT);
        FLO_INFO("MiB\n");

        FLO_INFO("DATA SIZE: ");
        FLO_INFO(options.data_size / ALIGNMENT);
        FLO_INFO("MiB\n");

        FLO_INFO("PADDING: ");
        FLO_INFO(padding / ALIGNMENT);
        FLO_INFO("MiB\n");

        FLO_INFO("IMAGE SIZE: ");
        FLO_INFO(image_size / ALIGNMENT);
        FLO_INFO("MiB\n");
    }

    srand((uint32_t)time(NULL));

    write_mbr(image);

    write_gpts(image);

    write_esp(image);

    if (options.num_esp_file_paths > 0) {
        // Add file paths to EFI System Partition
        for (uint32_t i = 0; i < options.num_esp_file_paths; i++) {
            if (!add_path_to_esp(options.esp_file_paths[i],
                                 options.esp_files[i], image)) {
                fprintf(stderr, "ERROR: Could not add '%s' to ESP\n",
                        options.esp_file_paths[i]);
            }
            fclose(options.esp_files[i]);
        }
    }

    if (options.num_data_files > 0) {
        // Add file paths to Basic Data Partition
        for (uint32_t i = 0; i < options.num_data_files; i++) {
            if (!add_file_to_data_partition(options.data_files[i], image)) {
                fprintf(stderr,
                        "ERROR: Could not add file '%s' to data partition\n",
                        options.data_files[i]);
            }
        }

        char info_file[12] = "DATAFLS.INF"; // "Data (partition) files info"
        char info_path[25] = {0};
        strcpy(info_path, "/EFI/BOOT/DATAFLS.INF");

        fp = fopen(info_file, "rbe");
        if (!fp) {
            fprintf(stderr, "ERROR: Could not open '%s'\n", info_file);
            return;
        }

        if (!add_path_to_esp(info_path, fp, image)) {
            fprintf(stderr, "ERROR: Could not add '%s' to ESP\n", info_path);
            return;
        }
        fclose(fp);
    }

    // Pad file to next 4KiB aligned size
    fseek(image, 0, SEEK_END);
    uint64_t current_size = ftell(image);
    uint64_t new_size = current_size - (current_size % 4096) + 4096;
    uint8_t byte = 0;

    // No vhd footer
    fseek(image, new_size - 1, SEEK_SET);
    checkedFwrite(&byte, 1, image);

    // Add disk image info file to hold at minimum the size of this disk image;
    //   this could be used in an EFI application later as part of an installer,
    //   for example
    image_size = (uint32_t)new_size; // Image size is used to write info file
    if (!add_disk_image_info_file(image)) {
        fprintf(stderr, "Error: Could not add disk image info file to '%s'\n",
                options.image_name);
    }

    // File cleanup
    fclose(image);
}

// =============================
// MAIN
// =============================
int main(int argc, char *argv[]) {
    if (__builtin_setjmp(fileCloser)) {
        for (ptrdiff_t i = 0; i < openedFiles.len; i++) {
            if (fclose(openedFiles.buf[i])) {
                FLO_FLUSH_AFTER(FLO_STDERR) {
                    FLO_ERROR(FLO_STRING("Failed to close FILE*\n"));
                    FLO_APPEND_ERRNO
                }
            }
        }
        return 1;
    }

    char *begin = mmap(NULL, memoryCap, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (begin == MAP_FAILED) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(FLO_STRING("Failed to allocate memory!\n"));
            FLO_APPEND_ERRNO
        }
        return 1;
    }

    flo_arena arena = (flo_arena){
        .beg = begin, .cap = memoryCap, .end = begin + (ptrdiff_t)(memoryCap)};

    if (__builtin_setjmp(memoryErrors)) {
        if (munmap(arena.beg, arena.cap) == -1) {
            FLO_FLUSH_AFTER(FLO_STDERR) {
                FLO_ERROR((FLO_STRING("Failed to unmap memory from"
                                      "arena !\n "
                                      "Arena Details:\n"
                                      "  beg: ")));
                FLO_ERROR(arena.beg);
                FLO_ERROR((FLO_STRING("\n end: ")));
                FLO_ERROR(arena.end);
                FLO_ERROR((FLO_STRING("\n cap: ")));
                FLO_ERROR(arena.cap);
                FLO_ERROR((FLO_STRING("\nZeroing Arena regardless.\n")));
            }
        }
        arena.beg = NULL;
        arena.end = NULL;
        arena.cap = 0;
        arena.jmp_buf = NULL;
        FLO_ERROR(
            (FLO_STRING("Early exit due to error or OOM/overflow in arena!\n")),
            FLO_FLUSH);
        __builtin_longjmp(fileCloser, 1);
    }
    arena.jmp_buf = memoryErrors;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            FLO_FLUSH_AFTER(FLO_STDOUT) {
                FLO_INFO(argv[0]);
                FLO_INFO(FLO_STRING(" [options]\n\noptions:\n"));

                // add data files
                FLO_INFO(
                    FLO_STRING("-ad --add-data-files\tAdd local files to the "
                               "basic data partition, annd create a\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("<DATAFLS.INF> file in the directory "
                                    "'/EFI/BOOT/' in the ESP.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING(
                    "This INF will hold info for each file added.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(
                    FLO_STRING("e.g: '-ad info.txt ../folderA/kernel.bin'.\n"));

                // add esp files
                FLO_INFO(FLO_STRING("-ae --add-esp-files\tAdd local files to "
                                    "the generated EFI System Partition.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING(
                    "File paths must start under root '/' and end with a \n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("slash '/', and all dir/file names are "
                                    "limited to FAT 8.3\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("naming. Each file is added in 2 parts; "
                                    "The 1st arg for\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING(
                    "the path, and the 2nd arg for the file to add to that\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("path. ex: '-ae /EFI/BOOT/ file1.txt' "
                                    "will add the local\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("file 'file1.txt' to the ESP under the "
                                    "path '/EFI/BOOT/'.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING(
                    "To add multiple files (up to 10), use multiple\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("<path> <file> args.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING(
                    "ex: '-ae /DIR1/ FILE1.TXT /DIR2/ FILE2.TXT'.\n"));

                // set data size
                FLO_INFO(FLO_STRING("-ds --data-size\tSet the size of the "
                                    "Basic Data Partition in MiB; Minimum\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("size is 1 MiB\n"));

                // set esp size
                FLO_INFO(FLO_STRING("-es --esp-size\t\tSet the size of the "
                                    "EFI System Partition in MiB\n"));

                // set lba size
                FLO_INFO(FLO_STRING("-l --lba-size\t\tSet the lba (sector) "
                                    "size in bytes; This is \n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("experimental, as tools are lacking for "
                                    "proper testing.\n"));
                FLO_INFO(FLO_STRING("\t\t\t"));
                FLO_INFO(FLO_STRING("experimental, as tools are lacking Valid "
                                    "sizes: 512/1024/2048/4096\n"));

                // set image name
                FLO_INFO(FLO_STRING("-i --image-name\t\tSet the image name. "
                                    "Default name is 'test.hdd'\n"));

                // help
                FLO_INFO(FLO_STRING("-h --help\t\tPrint this help text\n"));
            }
            return 1;
        }

        if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--image-name")) {
            if (++i >= argc) {
                return 1;
            }

            options.image_name = argv[i];
            continue;
        }

        if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--lba-size")) {
            if (++i >= argc) {
                return 1;
            }

            uint16_t lba_size = (uint16_t)strtol(argv[i], NULL, 10);

            if (lba_size != 512 && lba_size != 1024 && lba_size != 2048 &&
                lba_size != 4096) {
                FLO_FLUSH_AFTER(FLO_STDERR) {
                    FLO_ERROR(FLO_STRING("Error: Invalid LBA size of "));
                    FLO_ERROR(lba_size);
                    FLO_ERROR(
                        FLO_STRING(", must be one of 512/1024/2048/4096\n"));
                }

                return 1;
            }

            options.lba_size = lba_size;

            continue;
        }

        if (!strcmp(argv[i], "-es") || !strcmp(argv[i], "--esp-size")) {
            if (++i >= argc) {
                return 1;
            }

            options.esp_size = ALIGNMENT * (uint32_t)strtol(argv[i], NULL, 10);

            continue;
        }

        if (!strcmp(argv[i], "-ds") || !strcmp(argv[i], "--data-size")) {
            if (++i >= argc) {
                return 1;
            }

            options.data_size = ALIGNMENT * (uint32_t)strtol(argv[i], NULL, 10);
            continue;
        }

        if (!strcmp(argv[i], "-ae") || !strcmp(argv[i], "--add-esp-files")) {
            if (i + 2 >= argc) {
                FLO_FLUSH_AFTER(FLO_STDERR) {
                    FLO_ERROR(FLO_STRING("Error: Must include at least 1 path "
                                         "and 1 file to add to ESP\n"));
                }

                return 1;
            }

            // Allocate memory for file paths & File pointers
            // TODO: this code is dumb

            for (i += 1; i < argc && argv[i][0] != '-'; i++) {
                // Grab next 2 args, 1st will be path to add, 2nd will be
                // file to add to path
                // Get path to add
                strncpy(options.esp_file_paths[options.num_esp_file_paths],
                        argv[i], MAX_FILE_LEN - 1);

                // Ensure path starts and ends with a slash '/'
                if ((argv[i][0] != '/') ||
                    (argv[i][strlen(argv[i]) - 1] != '/')) {
                    FLO_FLUSH_AFTER(FLO_STDERR) {
                        FLO_ERROR(FLO_STRING(
                            "Error: All file paths to add to ESP must "
                            "start and end with slash '/'\n"));
                    }
                    return 1;
                }

                // Get FILE * for file to add to path
                i++;
                options.esp_files[options.num_esp_file_paths] =
                    fopen(argv[i], "rbe");
                if (!options.esp_files[options.num_esp_file_paths]) {
                    FLO_FLUSH_AFTER(FLO_STDERR) {
                        FLO_ERROR(FLO_STRING("Error: Could not fopen file "));
                        FLO_ERROR(argv[i], FLO_NEWLINE);
                    }

                    return 1;
                }

                // Concat file to add to path
                char *slash = strrchr(argv[i], '/');
                if (!slash) {
                    // Plain file name, no folder path
                    strncat(options.esp_file_paths[options.num_esp_file_paths],
                            argv[i], MAX_FILE_LEN - 1);
                } else {
                    // Get only last name in path, no folders
                    strncat(options.esp_file_paths[options.num_esp_file_paths],
                            slash + 1, // File name starts after final slash
                            MAX_FILE_LEN - 1);
                }

                if (++options.num_esp_file_paths == MAX_FILES) {
                    FLO_FLUSH_AFTER(FLO_STDERR) {
                        FLO_ERROR(FLO_STRING(
                            "Error: Number of ESP files to add must be <= "));
                        FLO_ERROR(MAX_FILES, FLO_NEWLINE);
                    }

                    return 1;
                }
            }

            // Overall for loop will increment i; in order to get next
            // option, decrement here
            i--;
            continue;
        }

        if (!strcmp(argv[i], "-ad") || !strcmp(argv[i], "--add-data-files")) {
            // Add files to the Basic Data Partition
            // Allocate memory for file paths

            for (i += 1; i < argc && argv[i][0] != '-'; i++) {
                // Grab next 2 args, 1st will be path to add, 2nd will be
                // file to add to path
                // Get path to add
                strncpy(options.data_files[options.num_data_files], argv[i],
                        MAX_FILE_LEN - 1);

                if (++options.num_data_files == MAX_FILES) {
                    FLO_FLUSH_AFTER(FLO_STDERR) {
                        FLO_ERROR(FLO_STRING("Error: Number of Data Parition "
                                             "files to add must be <= "));
                        FLO_ERROR(MAX_FILES, FLO_NEWLINE);
                    }

                    return 1;
                }
            }

            // Overall for loop will increment i; in order to get next
            // option, decrement here
            i--;
            continue;
        }
    }

    if ((options.lba_size == 512 && options.esp_size < 33) ||
        (options.lba_size == 1024 && options.esp_size < 65) ||
        (options.lba_size == 2048 && options.esp_size < 129) ||
        (options.lba_size == 4096 && options.esp_size < 257)) {
        FLO_FLUSH_AFTER(FLO_STDERR) {
            FLO_ERROR(
                FLO_STRING("Error: ESP Must be a minimum of 33/65/129/257 MiB "
                           "for LBA sizes 512/1024/2048/4096 respectively\n"));
        }

        return 1;
    }

    writeUEFIImage(arena);

    return 0;
}
