#include "test-framework/test.h"
#include "log/log.h"
#include "text/string.h" // for STRING, string
#include "util/assert.h" // for ASSERT

typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_RESET,
    COLOR_NUMS
} AnsiColor;

static string ansiColorToCode[COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

void appendColor(AnsiColor color) {
    appendToFlushBuffer(ansiColorToCode[color], 0);
}
void appendColorReset() {
    appendToFlushBuffer(ansiColorToCode[COLOR_RESET], 0);
}

typedef struct {
    U64 successes;
    U64 failures;
    string topic;
} TestTopic;

#define MAX_TEST_TOPICS 1 << 6

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
    FLUSH_AFTER {
        appendSpaces();

        LOG((STRING("[ ")));
        LOG(successes);
        LOG((STRING(" / ")));
        LOG(failures + successes);
        LOG((STRING(" ]\n")));
    }
}

void testSuiteStart(string mainTopic) {
    FLUSH_AFTER {
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
        FLUSH_AFTER {
            LOG((STRING("\nTest suite ")));
            appendColor(COLOR_RED);
            LOG((STRING("failed")));
            appendColorReset();
            LOG((STRING(".\n")));
        }
    } else {
        FLUSH_AFTER {
            LOG((STRING("\nTest suite ")));
            appendColor(COLOR_GREEN);
            LOG((STRING("successful")));
            appendColorReset();
            LOG((STRING(".\n")));
        }
    }

    return globalFailures > 0;
}

void testTopicStart(string testTopic) {
    addTopic(testTopic);

    FLUSH_AFTER {
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
    FLUSH_AFTER {
        appendSpaces();
        LOG((STRING("- ")));
        LOG(stringWithMinSizeDefault(testName, 50));
    }
}

void testSuccess() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].successes++;
    }

    FLUSH_AFTER {
        appendColor(COLOR_GREEN);
        LOG(stringWithMinSizeDefault(STRING("Success"), 20));
        appendColorReset();
        LOG((STRING("\n")));
    }
}

void testFailure() {
    for (U64 i = 0; i < nextTestTopic; i++) {
        testTopics[i].failures++;
    }

    FLUSH_AFTER {
        appendColor(COLOR_RED);
        LOG(stringWithMinSizeDefault(STRING("Failure"), 20));
        appendColorReset();
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
