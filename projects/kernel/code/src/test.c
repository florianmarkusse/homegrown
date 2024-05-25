#include "test.h"
#include "kernel-parameters.h"
#include "memory/definitions.h"
#include "util/types.h"

void drawBar() {
    KernelParameters *kernelParameters =
        (KernelParameters *)KERNEL_PARAMS_START;

    for (uint64_t i = 0; i < 10000; i++) {
        ((uint64_t *)kernelParameters->fb.ptr)[i] = 0xFFFFFFFFFFFFFFFF;
    }
}
