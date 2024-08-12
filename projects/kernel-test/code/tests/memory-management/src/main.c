#include "test-framework/test.h"
#include <stdio.h>

int main() {
    testSuiteStart(STRING("memory-management"));

    TEST_TOPIC(STRING("Physical memory tests")) {
        TEST(STRING("Does it work")) { testSuccess(); }
    }

    return testSuiteFinish();
}
