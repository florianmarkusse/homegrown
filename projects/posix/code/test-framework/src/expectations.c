#include "posix/test-framework/expectations.h"

#include "posix/log.h"
#include "shared/log.h"
#include "shared/text/converter.h"

void appendExpectCodeWithString(U64 expected, string expectedString, U64 actual,
                                string actualString) {
    PLOG(stringWithMinSizeDefault(STRING("Expected"), 10));
    PLOG((STRING(": ")));
    PLOG(stringWithMinSizeDefault(U64ToStringDefault(expected), 4));
    PLOG((STRING(" - ")));
    PLOG(expectedString, NEWLINE);

    PLOG(stringWithMinSizeDefault(STRING("Actual"), 10));
    PLOG((STRING(": ")));
    PLOG(stringWithMinSizeDefault(U64ToStringDefault(actual), 4));
    PLOG((STRING(" - ")));
    PLOG(actualString, NEWLINE);
}

void appendExpectString(string expectedString, string actualString) {
    PLOG(stringWithMinSizeDefault(STRING("Expected string"), 20));
    PLOG((STRING(": ")));
    PLOG(expectedString, NEWLINE);

    PLOG(stringWithMinSizeDefault(STRING("Actual string"), 20));
    PLOG((STRING(": ")));
    PLOG(actualString, NEWLINE);
}

void appendExpectBool(bool expectedBool, bool actualBool) {
    PLOG(stringWithMinSizeDefault(STRING("Expected bool"), 20));
    PLOG((STRING(": ")));
    PLOG(expectedBool, NEWLINE);

    PLOG(stringWithMinSizeDefault(STRING("Actual bool"), 20));
    PLOG((STRING(": ")));
    PLOG(actualBool, NEWLINE);
}

void appendExpectPtrDiff(U64 expectedNumber, U64 actualNumber) {
    PLOG(stringWithMinSizeDefault(STRING("Expected number"), 20));
    PLOG((STRING(": ")));
    PLOG(expectedNumber, NEWLINE);

    PLOG(stringWithMinSizeDefault(STRING("Actual number"), 20));
    PLOG((STRING(": ")));
    PLOG(actualNumber, NEWLINE);
}
void appendExpectUint(U64 expectedNumber, U64 actualNumber) {
    PLOG(stringWithMinSizeDefault(STRING("Expected number"), 20));
    PLOG((STRING(": ")));
    PLOG(expectedNumber, NEWLINE);

    PLOG(stringWithMinSizeDefault(STRING("Actual number"), 20));
    PLOG((STRING(": ")));
    PLOG(actualNumber, NEWLINE);
}
