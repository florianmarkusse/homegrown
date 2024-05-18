#ifndef EFI_C_EFI_PROTOCOL_BLOCK_IO_H
#define EFI_C_EFI_PROTOCOL_BLOCK_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_BLOCK_IO_PROTOCOL_GUID                                           \
    C_EFI_GUID(0x964e5b21, 0x6459, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct {
    CEfiU32 MediaId;
    CEfiBool RemovableMedia;
    CEfiBool MediaPresent;
    CEfiBool LogicalPartition;
    CEfiBool ReadOnly;
    CEfiBool WriteCaching;
    CEfiU32 BlockSize;
    CEfiU32 IoAlign;
    CEfiLba LastBlock;
    CEfiLba LowestAlignedLba;                 // added in Revision 2
    CEfiU32 LogicalBlocksPerPhysicalBlock;    // added in Revision 2
    CEfiU32 OptimalTransferLengthGranularity; // added in Revision 3
} CEfiBlockIoMedia;

typedef struct CEfiBlockIoProtocol {
    CEfiU64 Revision;
    CEfiBlockIoMedia *Media;
    // Not implemented cause we not needed (yet)
    void *Reset;
    CEfiStatus(CEFICALL *readBlocks)(CEfiBlockIoProtocol *this_,
                                     CEfiU32 mediaID, CEfiLba startingLBA,
                                     CEfiUSize bufferSize, void *buffer);

    void *WriteBlocks;
    void *FlushBlocks;
} CEfiBlockIoProtocol;

#ifdef __cplusplus
}
#endif

#endif
