#include "posix/test-framework/test.h"
#include "platform-abstraction/assert.h"        // for ASSERT
#include "platform-abstraction/log.h" // for ASSERT
#include "posix/log/log.h"
#include "shared/text/string.h" // for STRING, string

typedef struct {
    U64 successes;
    U64 failures;
    string topic;
} TestTopic;

static constexpr auto MAX_TEST_TOPICS = 1 << 6;

static TestTopic testTopics[MAX_TEST_TOPICS];
static U64 nextTestTopic = 0;

void addTopic(string topic) {
    ASSERT(nextTestTopic < MAX_TEST_TOPICS);
    testTopics[nextTestTopic++] =
        (TestTopic){.failures = 0, .successes = 0, .topic = topic};
}

void appendSpaces() {
    for (U64 i = 0; i < nextTestTopic - 1; i++) {
        LOG((STRING("  ")));
    }
}

void printTestScore(U64 successes, U64 failures) {
    FLUSH_AFTER(STDOUT) {
        appendSpaces();

        LOG((STRING("[ ")));
        LOG(successes);
        LOG((STRING(" / ")));
        LOG(failures + successes);
        LOG((STRING(" ]\n")));
    }
}

void testSuiteStart(string mainTopic) {
    FLUSH_AFTER(STDOUT) {
        LOG((STRING("Starting test suite for ")));
        LOG(mainTopic);
        LOG((STRING(" ...\n\n")));
    }

    addTopic(STRING("Root topic"));
}

int testSuiteFinish() {
    U64 globalSuccesses = testTopics[0].successes;
    U64 globalFailures = testTopics[0].failures;

    printTestScore(globalSuccesses, globalFailures);
    if (globalFailures > 0) {
        FLUSH_AFTER(STDERR) {
            LOG((STRING("\nTest suite ")));
            appendColor(COLOR_RED, STDERR);
            LOG((STRING("failed")));
            appendColorReset(STDERR);
            LOG((STRING(".\n")));
        }
    } else {
        FLUSH_AFTER(STDOUT) {
            LOG((STRING("\nTest suite ")));
            appendColor(COLOR_GREEN, STDOUT);
            LOG((STRING("successful")));
            appendColorReset(STDOUT);
            LOG((STRING(".\n")));
        }
    }

    return globalFailures > 0;
}

void testTopicStart(string testTopic) {
    addTopic(testTopic);

    FLUSH_AFTER(STDOUT) {
        appendSpaces();
        LOG((STRING("Testing ")));
        LOG(testTopic);
        LOG((STRING("...\n")));
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
        LOG((STRING("- ")));
        LOG(stringWithMinSizeDefault(testName, 50));
    }
}

void testSuccess() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].successes++;
    }

    FLUSH_AFTER(STDOUT) {
        appendColor(COLOR_GREEN, STDOUT);
        LOG(stringWithMinSizeDefault(STRING("Success"), 20));
        appendColorReset(STDOUT);
        LOG((STRING("\n")));
    }
}

void testFailure() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].failures++;
    }

    FLUSH_AFTER(STDERR) {
        appendColor(COLOR_RED, STDERR);
        LOG(stringWithMinSizeDefault(STRING("Failure"), 20));
        appendColorReset(STDERR);
        LOG((STRING("\n")));
    }
}

void appendTestFailureStart() {
    LOG((STRING("----------------------------------------------------"
                "----------------------------\n")));
    LOG((STRING("|                                    REASON         "
                "                           |\n")));
}

void appendTestFailureFinish() {
    LOG((STRING("|                                                   "
                "                           |\n")));
    LOG((STRING("----------------------------------------------------"
                "----------------------------\n")));
}
