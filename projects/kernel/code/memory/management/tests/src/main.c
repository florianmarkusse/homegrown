#include "test-framework/test.h"
#include "test/physical.h"
#include "test/virtual.h"

int main() {
    testSuiteStart(STRING("Memory Management"));

    testPhysicalMemoryManagement();
    testVirtualMemoryManagement();

    return testSuiteFinish();
}
