#ifndef POSIX_TEST_FRAMEWORK_EXPECTATIONS_H
#define POSIX_TEST_FRAMEWORK_EXPECTATIONS_H

#include "shared/text/string.h" // for string
#include "shared/types/types.h"

void appendExpectCodeWithString(U64 expected, string expectedString, U64 actual,
                                string actualString);
void appendExpectString(string expectedString, string actualString);
void appendExpectBool(bool expectedBool, bool actualBool);
void appendExpectPtrDiff(U64 expectedNumber, U64 actualNumber);
void appendExpectUint(U64 expectedNumber, U64 actualNumber);

#endif
