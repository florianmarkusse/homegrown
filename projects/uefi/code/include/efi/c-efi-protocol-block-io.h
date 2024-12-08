#ifndef EFI_C_EFI_PROTOCOL_BLOCK_IO_H
#define EFI_C_EFI_PROTOCOL_BLOCK_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "acpi/guid.h"
#include "c-efi-base.h"

static constexpr auto BLOCK_IO_PROTOCOL_GUID =
    (Guid){.ms1 = 0x964e5b21,
           .ms2 = 0x6459,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

typedef struct {
    U32 MediaId;
    bool RemovableMedia;
    bool MediaPresent;
    bool LogicalPartition;
    bool ReadOnly;
    bool WriteCaching;
    U32 BlockSize;
    U32 IoAlign;
    Lba LastBlock;
    Lba LowestAlignedLba;                 // added in Revision 2
    U32 LogicalBlocksPerPhysicalBlock;    // added in Revision 2
    U32 OptimalTransferLengthGranularity; // added in Revision 3
} BlockIoMedia;

typedef struct BlockIoProtocol {
    U64 Revision;
    BlockIoMedia *Media;
    // Not implemented cause we not needed (yet)
    void *Reset;
    Status(EFICALL *readBlocks)(BlockIoProtocol *this_, U32 mediaID,
                                Lba startingLBA, USize bufferSize,
                                void *buffer);

    void *WriteBlocks;
    void *FlushBlocks;
} BlockIoProtocol;

#ifdef __cplusplus
}
#endif

#endif
