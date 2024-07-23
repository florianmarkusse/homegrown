#include "test.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "types.h"

void drawBar() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    for (U64 i = 0; i < 10000; i++) {
        ((U64 *)kernelParameters->fb.ptr)[i] = 0xFFFFFFFFFFFFFFFF;
    }

    __asm__ __volatile__("hlt");
}
