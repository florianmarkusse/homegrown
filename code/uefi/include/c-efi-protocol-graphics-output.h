#ifdef __cplusplus
extern "C" {
#endif

#include <c-efi-base.h>
#include <c-efi-system.h>

#define C_EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID                                    \
    C_EFI_GUID(0x9042a9de, 0x23dc, 0x4a38, 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, \
               0x51, 0x6a)

typedef struct {
    CEfiU8 Blue;
    CEfiU8 Green;
    CEfiU8 Red;
    CEfiU8 Reserved;
} CEfiBltPixel;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} CEfiBltOperation;

typedef struct CEfiGraphicsOutputProtocolBlt {
    CEfiGraphicsOutputProtocol *this_;
    CEfiBltPixel *bltBuffer;
    CEfiBltOperation bltOperation;
    CEfiUSize sourceX;
    CEfiUSize sourceY;
    CEfiUSize destinationX;
    CEfiUSize destinationY;
    CEfiUSize width;
    CEfiUSize height;
    CEfiUSize delta;

} CEfiGraphicsOutputProtocolBlt;

typedef struct {
    CEfiU32 redMask;
    CEfiU32 greenMask;
    CEfiU32 blueMask;
    CEfiU32 reservedMask;
} CEfiPixelBitmask;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} CEfiGraphicsPixelFormat;

typedef struct {
    CEfiU32 version;
    CEfiU32 horizontalResolution;
    CEfiU32 verticalResolution;
    CEfiGraphicsPixelFormat pixelFormat;
    CEfiPixelBitmask pixelInformation;
    CEfiU32 pixelsPerScanLine;
} CEfiGraphicsOutputModeInformation;

typedef struct {
    CEfiU32 maxMode;
    CEfiU32 mode;
    CEfiGraphicsOutputModeInformation *info;
    CEfiUSize sizeOfInfo;
    CEfiPhysicalAddress frameBufferBase;
    CEfiUSize frameBufferSize;
} CEfiGraphicsOutputProtocolMode;

typedef struct CEfiGraphicsOutputProtocol {
    CEfiStatus(CEFICALL *queryMode)(CEfiGraphicsOutputProtocol *this_,
                                    CEfiU32 modeNumber, CEfiUSize *sizeOfInfo,
                                    CEfiGraphicsOutputModeInformation **info);
    CEfiStatus(CEFICALL *setMode)(CEfiGraphicsOutputProtocol *this_,
                                  CEfiU32 modeNumber);
    CEfiGraphicsOutputProtocolBlt blt;
    CEfiGraphicsOutputProtocolMode *mode;
} CEfiGraphicsOutputProtocol;

#ifdef __cplusplus
}
#endif
