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

extern uint8_t
    fontData[] asm("_binary__home_florian_Desktop_homegrown_projects_kernel_"
                   "code____resources_font_psf_start");

__attribute__((ms_abi, section("kernel-start"))) int
kernelmain(KernelParameters kernelParameters) {
    flo_setupScreen(
        (flo_ScreenDimension){.scanline = kernelParameters.fb.scanline,
                              .size = kernelParameters.fb.size,
                              .width = kernelParameters.fb.columns,

                              .height = kernelParameters.fb.rows,
                              .buffer = (uint32_t *)kernelParameters.fb.ptr});

    while (1) {
        ;
    }
}
