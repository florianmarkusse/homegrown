#include "posix/test-framework/expectations.h"
#include "platform-abstraction/log/log.h" // for ERROR, LOG_CHOOSER_IMPL_2
#include "posix/log/log.h"

void appendExpectCodeWithString(U64 expected, string expectedString, U64 actual,
                                string actualString) {
    LOG(stringWithMinSizeDefault(STRING("Expected"), 10));
    LOG((STRING(": ")));
    LOG(stringWithMinSizeDefault(U64ToStringDefault(expected), 4));
    LOG((STRING(" - ")));
    LOG(expectedString, NEWLINE);

    LOG(stringWithMinSizeDefault(STRING("Actual"), 10));
    LOG((STRING(": ")));
    LOG(stringWithMinSizeDefault(U64ToStringDefault(actual), 4));
    LOG((STRING(" - ")));
    LOG(actualString, NEWLINE);
}

void appendExpectString(string expectedString, string actualString) {
    LOG(stringWithMinSizeDefault(STRING("Expected string"), 20));
    LOG((STRING(": ")));
    LOG(expectedString, NEWLINE);

    LOG(stringWithMinSizeDefault(STRING("Actual string"), 20));
    LOG((STRING(": ")));
    LOG(actualString, NEWLINE);
}

void appendExpectBool(bool expectedBool, bool actualBool) {
    LOG(stringWithMinSizeDefault(STRING("Expected bool"), 20));
    LOG((STRING(": ")));
    LOG(expectedBool, NEWLINE);

    LOG(stringWithMinSizeDefault(STRING("Actual bool"), 20));
    LOG((STRING(": ")));
    LOG(actualBool, NEWLINE);
}

void appendExpectPtrDiff(U64 expectedNumber, U64 actualNumber) {
    LOG(stringWithMinSizeDefault(STRING("Expected number"), 20));
    LOG((STRING(": ")));
    LOG(expectedNumber, NEWLINE);

    LOG(stringWithMinSizeDefault(STRING("Actual number"), 20));
    LOG((STRING(": ")));
    LOG(actualNumber, NEWLINE);
}
void appendExpectUint(U64 expectedNumber, U64 actualNumber) {
    LOG(stringWithMinSizeDefault(STRING("Expected number"), 20));
    LOG((STRING(": ")));
    LOG(expectedNumber, NEWLINE);

    LOG(stringWithMinSizeDefault(STRING("Actual number"), 20));
    LOG((STRING(": ")));
    LOG(actualNumber, NEWLINE);
}
