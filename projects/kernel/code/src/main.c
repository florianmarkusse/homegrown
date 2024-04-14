#include "util/log.h"
#include "util/types.h"

typedef struct {
    uint32_t columns;
    uint32_t rows;
    uint32_t scanline;
    uint64_t ptr;
    uint64_t size;
} FrameBuffer;

typedef struct {
    uint64_t ptr;
    uint64_t size;
} MemoryMap;

typedef struct {
    FrameBuffer fb;
    MemoryMap *memory;
} KernelParameters;

__attribute__((ms_abi, section("kernel-start"))) int
kernelmain(KernelParameters kernelParameters) {
    flo_setupScreen(
        (flo_ScreenDimension){.scanline = kernelParameters.fb.scanline,
                              .size = kernelParameters.fb.size,
                              .width = kernelParameters.fb.columns,
                              .height = kernelParameters.fb.rows,
                              .buffer = (uint32_t *)kernelParameters.fb.ptr});

    // flo_printToScreen(FLO_STRING("H"), 0);

    //  uint32_t *fb = (uint32_t *)kernelParameters.fb.ptr;
    //  uint32_t xres = kernelParameters.fb.scanline;
    //  uint32_t yres = kernelParameters.fb.columns;

    //  // Clear screen to solid color
    //  for (uint32_t y = 0; y < yres; y++) {
    //      for (uint32_t x = 0; x < xres; x++) {
    //          fb[y * xres + x] = 0xFFDDDDDD; // Light Gray AARRGGBB 8888
    //      }
    //  }

    //  // Draw square in top left
    //  for (uint32_t y = 0; y < yres / 5; y++) {
    //      for (uint32_t x = 0; x < xres / 5; x++) {
    //          fb[y * xres + x] = 0xFFCC2222; // AARRGGBB 8888
    //      }
    //  }

    while (1) {
        ;
    }
}
