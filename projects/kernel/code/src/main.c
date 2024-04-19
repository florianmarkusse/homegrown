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

#define KERNEL_PARAMS_START 0xfffffffff7000000
__attribute__((ms_abi, section("kernel-start"))) int kernelmain() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;
    flo_setupScreen(
        (flo_ScreenDimension){.scanline = kernelParameters->fb.scanline,
                              .size = kernelParameters->fb.size,
                              .width = kernelParameters->fb.columns,
                              .height = kernelParameters->fb.rows,
                              .buffer = (uint32_t *)kernelParameters->fb.ptr});

    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), FLO_NEWLINE);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);
    flo_printToScreen(FLO_STRING("Hello ther"), 0);

    while (1) {
        ;
    }
}
