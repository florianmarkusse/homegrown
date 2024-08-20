#include "quickie/hello.h"
#include "test-framework/test.h"
#include "text/string.h"

int main() {
    testSuiteStart(STRING("Quickie tests"));

    TEST_TOPIC(STRING("Topic 1")) {
        TEST(STRING("test 1")) { testSuccess(); }

        TEST(STRING("test 2")) { testSuccess(); }
    }

    TEST_TOPIC(STRING("Topic 2")) {
        TEST(STRING("test 1")) { testSuccess(); }

        TEST(STRING("test 2")) { testSuccess(); }
    }

    TEST_TOPIC(STRING("Topic 3")) {
        TEST(STRING("test 1")) { testSuccess(); }

        TEST(STRING("test 2")) { testSuccess(); }
        TEST(STRING("test 3")) { testSuccess(); }
        TEST(STRING("test 4")) { testSuccess(); }
    }

    return testSuiteFinish();
}
