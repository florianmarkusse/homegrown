#include "test-framework/test.h"
#include "util/assert.h"      // for ASSERT
#include "util/log.h"         // for LOG_CHOOSER_IMPL_2, INFO, ...
#include "util/text/string.h" // for STRING, string
#include <stddef.h>           // for ptrdiff_t
#include <stdint.h>           // for uint64_t

typedef struct {
    uint64_t successes;
    uint64_t failures;
    string topic;
} TestTopic;

#define MAX_TEST_TOPICS 1 << 6

static TestTopic testTopics[MAX_TEST_TOPICS];
static ptrdiff_t nextTestTopic = 0;

void addTopic(string topic) {
    ASSERT(nextTestTopic < MAX_TEST_TOPICS);
    testTopics[nextTestTopic++] =
        (TestTopic){.failures = 0, .successes = 0, .topic = topic};
}

void appendSpaces() {
    for (ptrdiff_t i = 0; i < nextTestTopic - 1; i++) {
        INFO((STRING("  ")));
    }
}

void printTestScore(uint64_t successes, uint64_t failures) {
    FLUSH_AFTER(STDOUT) {
        appendSpaces();

        INFO((STRING("[ ")));
        INFO(successes);
        INFO((STRING(" / ")));
        INFO(failures + successes);
        INFO((STRING(" ]\n")));
    }
}

void testSuiteStart() {
    INFO((STRING("Starting test suite...\n\n")), FLUSH);

    addTopic(STRING("Root topic"));
}

int testSuiteFinish() {
    uint64_t globalSuccesses = testTopics[0].successes;
    uint64_t globalFailures = testTopics[0].failures;

    printTestScore(globalSuccesses, globalFailures);
    if (globalFailures > 0) {
        FLUSH_AFTER(STDERR) {
            ERROR((STRING("\nTest suite ")));
            appendColor(COLOR_RED, STDERR);
            ERROR((STRING("failed")));
            appendColorReset(STDERR);
            ERROR((STRING(".\n")));
        }
    } else {
        FLUSH_AFTER(STDOUT) {
            INFO((STRING("\nTest suite ")));
            appendColor(COLOR_GREEN, STDOUT);
            INFO((STRING("successful")));
            appendColorReset(STDOUT);
            INFO((STRING(".\n")));
        }
    }

    return globalFailures > 0;
}

void testTopicStart(string testTopic) {
    addTopic(testTopic);

    FLUSH_AFTER(STDOUT) {
        appendSpaces();
        INFO((STRING("Testing ")));
        INFO(testTopic);
        INFO((STRING("...\n")));
    }
}

void testTopicFinish() {
    printTestScore(testTopics[nextTestTopic - 1].successes,
                       testTopics[nextTestTopic - 1].failures);

    nextTestTopic--;
}

void unitTestStart(string testName) {
    FLUSH_AFTER(STDOUT) {
        appendSpaces();
        INFO((STRING("- ")));
        INFO(stringWithMinSizeDefault(testName, 50));
    }
}

void testSuccess() {
    for (ptrdiff_t i = 0; i < nextTestTopic; i++) {
        testTopics[i].successes++;
    }

    FLUSH_AFTER(STDOUT) {
        appendColor(COLOR_GREEN, STDOUT);
        INFO(stringWithMinSizeDefault(STRING("Success"), 20));
        appendColorReset(STDOUT);
        INFO((STRING("\n")));
    }
}

void testFailure() {
    for (ptrdiff_t i = 0; i < nextTestTopic; i++) {
        testTopics[i].failures++;
    }

    FLUSH_AFTER(STDERR) {
        appendColor(COLOR_RED, STDERR);
        ERROR(stringWithMinSizeDefault(STRING("Failure"), 20));
        appendColorReset(STDERR);
        ERROR((STRING("\n")));
    }
}

void appendTestFailureStart() {
    ERROR((STRING("----------------------------------------------------"
                          "----------------------------\n")));
    ERROR((STRING("|                                    REASON         "
                          "                           |\n")));
}

void appendTestFailureFinish() {
    ERROR((STRING("|                                                   "
                          "                           |\n")));
    ERROR((STRING("----------------------------------------------------"
                          "----------------------------\n")));
}
