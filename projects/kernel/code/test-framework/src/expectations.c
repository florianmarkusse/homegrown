#include "test-framework/expectations.h"
#include "util/log.h"

void appendExpectCodeWithString(ptrdiff_t expected,
                                    string expectedString, ptrdiff_t actual,
                                    string actualString) {
    ERROR(stringWithMinSizeDefault(STRING("Expected"), 10));
    ERROR((STRING(": ")));
    ERROR(
        stringWithMinSizeDefault(ptrdiffToStringDefault(expected), 4));
    ERROR((STRING(" - ")));
    ERROR(expectedString, NEWLINE);

    ERROR(stringWithMinSizeDefault(STRING("Actual"), 10));
    ERROR((STRING(": ")));
    ERROR(
        stringWithMinSizeDefault(ptrdiffToStringDefault(actual), 4));
    ERROR((STRING(" - ")));
    ERROR(actualString, NEWLINE);
}

void appendExpectString(string expectedString,
                            string actualString) {
    ERROR(stringWithMinSizeDefault(STRING("Expected string"), 20));
    ERROR((STRING(": ")));
    ERROR(expectedString, NEWLINE);

    ERROR(stringWithMinSizeDefault(STRING("Actual string"), 20));
    ERROR((STRING(": ")));
    ERROR(actualString, NEWLINE);
}

void appendExpectBool(bool expectedBool, bool actualBool) {
    ERROR(stringWithMinSizeDefault(STRING("Expected bool"), 20));
    ERROR((STRING(": ")));
    ERROR(expectedBool, NEWLINE);

    ERROR(stringWithMinSizeDefault(STRING("Actual bool"), 20));
    ERROR((STRING(": ")));
    ERROR(actualBool, NEWLINE);
}

void appendExpectPtrDiff(ptrdiff_t expectedNumber, ptrdiff_t actualNumber) {
    ERROR(stringWithMinSizeDefault(STRING("Expected number"), 20));
    ERROR((STRING(": ")));
    ERROR(expectedNumber, NEWLINE);

    ERROR(stringWithMinSizeDefault(STRING("Actual number"), 20));
    ERROR((STRING(": ")));
    ERROR(actualNumber, NEWLINE);
}
void appendExpectUint(uint64_t expectedNumber, uint64_t actualNumber) {
    ERROR(stringWithMinSizeDefault(STRING("Expected number"), 20));
    ERROR((STRING(": ")));
    ERROR(expectedNumber, NEWLINE);

    ERROR(stringWithMinSizeDefault(STRING("Actual number"), 20));
    ERROR((STRING(": ")));
    ERROR(actualNumber, NEWLINE);
}
