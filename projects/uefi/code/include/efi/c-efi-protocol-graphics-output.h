#ifndef EFI_C_EFI_PROTOCOL_GRAPHICS_OUTPUT_H
#define EFI_C_EFI_PROTOCOL_GRAPHICS_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID                                    \
    C_EFI_GUID(0x9042a9de, 0x23dc, 0x4a38, 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, \
               0x51, 0x6a)

typedef struct {
    U8 Blue;
    U8 Green;
    U8 Red;
    U8 Reserved;
} CEfiBltPixel;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} CEfiBltOperation;

typedef struct {
    U32 redMask;
    U32 greenMask;
    U32 blueMask;
    U32 reservedMask;
} CEfiPixelBitmask;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} CEfiGraphicsPixelFormat;

typedef struct {
    U32 version;
    U32 horizontalResolution;
    U32 verticalResolution;
    CEfiGraphicsPixelFormat pixelFormat;
    CEfiPixelBitmask pixelInformation;
    U32 pixelsPerScanLine;
} CEfiGraphicsOutputModeInformation;

typedef struct {
    U32 maxMode;
    U32 mode;
    CEfiGraphicsOutputModeInformation *info;
    CEfiUSize sizeOfInfo;
    CEfiPhysicalAddress frameBufferBase;
    CEfiUSize frameBufferSize;
} CEfiGraphicsOutputProtocolMode;

typedef struct CEfiGraphicsOutputProtocol {
    CEfiStatus(CEFICALL *queryMode)(CEfiGraphicsOutputProtocol *this_,
                                    U32 modeNumber, CEfiUSize *sizeOfInfo,
                                    CEfiGraphicsOutputModeInformation **info);
    CEfiStatus(CEFICALL *setMode)(CEfiGraphicsOutputProtocol *this_,
                                  U32 modeNumber);
    CEfiStatus(CEFICALL *blt)(CEfiGraphicsOutputProtocol *this_,
                              CEfiBltPixel *bltBuffer,
                              CEfiBltOperation bltOperation, CEfiUSize sourceX,
                              CEfiUSize sourceY, CEfiUSize destinationX,
                              CEfiUSize destinationY, CEfiUSize width,
                              CEfiUSize height, CEfiUSize delta);
    CEfiGraphicsOutputProtocolMode *mode;
} CEfiGraphicsOutputProtocol;

#ifdef __cplusplus
}
#endif

#endif
