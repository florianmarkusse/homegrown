#include "posix/test-framework/test.h"
#include "test/physical.h"
#include "shared/text/string.h"

int main() {
    testSuiteStart(STRING("Memory Management"));

    testPhysicalMemoryManagement();

    return testSuiteFinish();
}
