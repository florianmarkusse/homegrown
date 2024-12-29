#ifndef EFI_EFI_C_EFI_PROTOCOL_GRAPHICS_OUTPUT_H
#define EFI_EFI_C_EFI_PROTOCOL_GRAPHICS_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uefi/guid.h"
#include "efi/efi/c-efi-base.h"

static constexpr auto GRAPHICS_OUTPUT_PROTOCOL_GUID =
    (Guid){.ms1 = 0x9042a9de,
           .ms2 = 0x23dc,
           .ms3 = 0x4a38,
           .ms4 = {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

typedef struct {
    U8 Blue;
    U8 Green;
    U8 Red;
    U8 Reserved;
} BltPixel;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} BltOperation;

typedef struct {
    U32 redMask;
    U32 greenMask;
    U32 blueMask;
    U32 reservedMask;
} PixelBitmask;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} GraphicsPixelFormat;

typedef struct {
    U32 version;
    U32 horizontalResolution;
    U32 verticalResolution;
    GraphicsPixelFormat pixelFormat;
    PixelBitmask pixelInformation;
    U32 pixelsPerScanLine;
} GraphicsOutputModeInformation;

typedef struct {
    U32 maxMode;
    U32 mode;
    GraphicsOutputModeInformation *info;
    USize sizeOfInfo;
    PhysicalAddress frameBufferBase;
    USize frameBufferSize;
} GraphicsOutputProtocolMode;

typedef struct GraphicsOutputProtocol {
    Status(EFICALL *queryMode)(GraphicsOutputProtocol *this_, U32 modeNumber,
                               USize *sizeOfInfo,
                               GraphicsOutputModeInformation **info);
    Status(EFICALL *setMode)(GraphicsOutputProtocol *this_, U32 modeNumber);
    Status(EFICALL *blt)(GraphicsOutputProtocol *this_, BltPixel *bltBuffer,
                         BltOperation bltOperation, USize sourceX,
                         USize sourceY, USize destinationX, USize destinationY,
                         USize width, USize height, USize delta);
    GraphicsOutputProtocolMode *mode;
} GraphicsOutputProtocol;

#ifdef __cplusplus
}
#endif

#endif
