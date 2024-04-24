#include "util/log.h"
#include "util/types.h"

typedef struct {
    uint64_t ptr;
    uint64_t size;
    uint32_t columns;
    uint32_t rows;
    uint32_t scanline;
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
    //    __asm__ __volatile__("movq $0x00FF00FF, %%rax;" // Load the absolute
    //    value
    //                         "movq %%rax, (%%rdx);"     // Store the value at
    //                         the
    //                                                    // address pointed to
    //                                                    by
    //                         "hlt;" ::"d"(*(uint32_t *)KERNEL_PARAMS_START)
    //                         :);

    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;
    setupScreen(
        (ScreenDimension){.scanline = kernelParameters->fb.scanline,
                          .size = kernelParameters->fb.size,
                          .width = kernelParameters->fb.columns,
                          .height = kernelParameters->fb.rows,
                          .buffer = (uint32_t *)kernelParameters->fb.ptr});

    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), NEWLINE);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);
    //   printToScreen(STRING("Hello ther"), 0);

    while (1) {
        ;
    }
}
