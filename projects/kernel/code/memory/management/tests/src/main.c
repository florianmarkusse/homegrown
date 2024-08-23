#include "cpu/idt.h"
#include "log/log.h"
#include "memory/management/physical.h"
#include "test-framework/test.h"
#include "text/string.h"

int main() {
    testSuiteStart(STRING("Memory Management"));

    TEST_TOPIC(STRING("Physical")) {
        TEST(STRING("test 1")) {
            if (1234 != 1234) {
                TEST_FAILURE {
                    LOG(STRING("The return value is not equal to 123\n"));
                }

                break;
            }

            if (!isFaultTriggered(FAULT_NO_MORE_PHYSICAL_MEMORY)) {
                TEST_FAILURE { LOG(STRING("Fault was not triggered!!!\n")); }
                break;
            }
            testSuccess();
        }
    }

    return testSuiteFinish();
}
