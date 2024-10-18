#include "interoperation/memory/sizes.h"
#include "posix/log/log.h"
#include "posix/test-framework/test.h"
#include "shared/memory/allocator/arena.h"
#include "test/physical.h"

int main() {
    testSuiteStart(STRING("Memory Management"));

    testPhysicalMemoryManagement();

    return testSuiteFinish();
}
