#include "interoperation/memory/sizes.h"
#include "log/log.h"
#include "memory/management/allocator/arena.h"
#include "test-framework/test.h"
#include "test/physical.h"
#include <stdlib.h>

int main() {
    void *logBuffer = malloc(2 * MiB);
    if (!logBuffer) {
        return 1;
    }

    void *jumper[5];
    if (__builtin_setjmp(jumper)) {
        return 1;
    }
    Arena arena = {.beg = logBuffer,
                   .curFree = logBuffer,
                   .end = logBuffer + (2 * MiB),
                   .jmp_buf = jumper};

    initLogger(&arena);

    testSuiteStart(STRING("Memory Management"));

    testPhysicalMemoryManagement();

    return testSuiteFinish();
}
