#ifndef TEST_FRAMEWORK_EXPECTATIONS_H
#define TEST_FRAMEWORK_EXPECTATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h"
#include "text/string.h" // for string

void appendExpectCodeWithString(U64 expected, string expectedString, U64 actual,
                                string actualString);
void appendExpectString(string expectedString, string actualString);
void appendExpectBool(bool expectedBool, bool actualBool);
void appendExpectPtrDiff(U64 expectedNumber, U64 actualNumber);
void appendExpectUint(U64 expectedNumber, U64 actualNumber);

#ifdef __cplusplus
}
#endif

#endif
