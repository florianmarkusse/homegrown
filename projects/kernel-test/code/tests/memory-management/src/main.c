#include "interoperation/types.h"
#include "quickie/hello.h"
#include "test-framework/test.h"

int main() {
    testSuiteStart(STRING("memory-management"));

    TEST_TOPIC(STRING("Physical memory tests")) {
        TEST(STRING("Does it work")) {
            U64 bigNumber = getBigNumber();
            if (bigNumber == 123456868654) {
                testSuccess();
            } else {
                testFailure();
            }
        }
    }

    return testSuiteFinish();
}
