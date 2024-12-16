#ifndef POSIX_TEST_FRAMEWORK_TEST_H
#define POSIX_TEST_FRAMEWORK_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "posix/log.h"
#include "shared/macros.h"      // for MACRO_VAR
#include "shared/text/string.h" // for string

void testSuiteStart(string mainTopic);
int testSuiteFinish();

void testTopicStart(string testTopic);
void testTopicFinish();

void unitTestStart(string testName);

void testSuccess();

void testFailure();
void appendTestFailureStart();
void appendTestFailureFinish();

#define TEST_FAILURE                                                           \
    for (auto MACRO_VAR(i) = (testFailure(), appendTestFailureStart(), 0);     \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = (appendTestFailureFinish(),          \
                                           PLOG(STRING("\n\n"), FLUSH), 1))

#define TEST(testString)                                                       \
    for (auto MACRO_VAR(i) = (unitTestStart(testString), 0); MACRO_VAR(i) < 1; \
         MACRO_VAR(i) = 1)

#define TEST_TOPIC(testTopicString)                                            \
    for (auto MACRO_VAR(i) = (testTopicStart(testTopicString), 0);             \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = (testTopicFinish(), 1))

#ifdef __cplusplus
}
#endif

#endif
