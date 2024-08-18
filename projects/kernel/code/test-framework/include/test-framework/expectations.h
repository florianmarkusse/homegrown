#ifndef TEST_FRAMEWORK_EXPECTATIONS_H
#define TEST_FRAMEWORK_EXPECTATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "text/string.h" // for string
#include <stdbool.h>          // for false, true, bool
#include <stddef.h>           // for ptrdiff_t
#include <stdint.h>           // for uint64_t

void appendExpectCodeWithString(ptrdiff_t expected,
                                    string expectedString, ptrdiff_t actual,
                                    string actualString);
void appendExpectString(string expectedString, string actualString);
void appendExpectBool(bool expectedBool, bool actualBool);
void appendExpectPtrDiff(ptrdiff_t expectedNumber, ptrdiff_t actualNumber);
void appendExpectUint(uint64_t expectedNumber, uint64_t actualNumber);

#ifdef __cplusplus
}
#endif

#endif
