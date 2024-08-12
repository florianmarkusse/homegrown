#include "test-framework/test.h"
#include <stdio.h>

int main() {
    testSuiteStart();

    TEST_TOPIC(STRING("Physical memory tests")) {
        TEST(STRING("Does it work")) {
            printf("hello world!b");
            testSuccess();
        }
    }

    return testSuiteFinish();
}
