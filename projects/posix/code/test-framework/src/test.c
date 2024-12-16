#include "posix/test-framework/test.h"

#include "posix/log.h"
#include "shared/assert.h"      // for ASSERT
#include "shared/text/string.h" // for STRING, string
#include "shared/text/converter.h"
#include "shared/types/types.h"

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
        PLOG((STRING("  ")));
    }
}

void printTestScore(U64 successes, U64 failures) {
    PFLUSH_AFTER(STDOUT) {
        appendSpaces();

        PLOG((STRING("[ ")));
        PLOG(successes);
        PLOG((STRING(" / ")));
        PLOG(failures + successes);
        PLOG((STRING(" ]\n")));
    }
}

void testSuiteStart(string mainTopic) {
    PFLUSH_AFTER(STDOUT) {
        PLOG((STRING("Starting test suite for ")));
        PLOG(mainTopic);
        PLOG((STRING(" ...\n\n")));
    }

    addTopic(STRING("Root topic"));
}

int testSuiteFinish() {
    U64 globalSuccesses = testTopics[0].successes;
    U64 globalFailures = testTopics[0].failures;

    printTestScore(globalSuccesses, globalFailures);
    if (globalFailures > 0) {
        PFLUSH_AFTER(STDERR) {
            PLOG((STRING("\nTest suite ")));
            appendColor(COLOR_RED, STDERR);
            PLOG((STRING("failed")));
            appendColorReset(STDERR);
            PLOG((STRING(".\n")));
        }
    } else {
        PFLUSH_AFTER(STDOUT) {
            PLOG((STRING("\nTest suite ")));
            appendColor(COLOR_GREEN, STDOUT);
            PLOG((STRING("successful")));
            appendColorReset(STDOUT);
            PLOG((STRING(".\n")));
        }
    }

    return globalFailures > 0;
}

void testTopicStart(string testTopic) {
    addTopic(testTopic);

    PFLUSH_AFTER(STDOUT) {
        appendSpaces();
        PLOG((STRING("Testing ")));
        PLOG(testTopic);
        PLOG((STRING("...\n")));
    }
}

void testTopicFinish() {
    printTestScore(testTopics[nextTestTopic - 1].successes,
                   testTopics[nextTestTopic - 1].failures);

    nextTestTopic--;
}

void unitTestStart(string testName) {
    PFLUSH_AFTER(STDOUT) {
        appendSpaces();
        PLOG((STRING("- ")));
        PLOG(stringWithMinSizeDefault(testName, 50));
    }
}

void testSuccess() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].successes++;
    }

    PFLUSH_AFTER(STDOUT) {
        appendColor(COLOR_GREEN, STDOUT);
        PLOG(stringWithMinSizeDefault(STRING("Success"), 20));
        appendColorReset(STDOUT);
        PLOG((STRING("\n")));
    }
}

void testFailure() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].failures++;
    }

    PFLUSH_AFTER(STDOUT) {
        appendColor(COLOR_RED, STDOUT);
        PLOG(stringWithMinSizeDefault(STRING("Failure"), 20));
        appendColorReset(STDOUT);
        PLOG((STRING("\n")));
    }
}

void appendTestFailureStart() {
    PLOG((STRING("----------------------------------------------------"
                 "----------------------------\n")));
    PLOG((STRING("|                                    REASON         "
                 "                           |\n")));
}

void appendTestFailureFinish() {
    PLOG((STRING("|                                                   "
                 "                           |\n")));
    PLOG((STRING("----------------------------------------------------"
                 "----------------------------\n")));
}
