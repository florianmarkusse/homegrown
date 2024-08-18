#ifndef TEST_FRAMEWORK_TEST_H
#define TEST_FRAMEWORK_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/log.h"    // for ERROR, FLUSH
#include "util/macros.h" // for MACRO_VAR
#include <stddef.h>
#include <text/string.h> // for string

#define TEST_FAILURE                                                           \
    for (ptrdiff_t MACRO_VAR(i) =                                              \
             (testFailure(), appendTestFailureStart(), 0);                     \
         MACRO_VAR(i) < 1;                                                     \
         MACRO_VAR(i) = (appendTestFailureFinish(), ERROR("\n\n", FLUSH), 1))

#define TEST(testString)                                                       \
    for (ptrdiff_t MACRO_VAR(i) = (unitTestStart(testString), 0);              \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = 1)

#define TEST_TOPIC(testTopicString)                                            \
    for (ptrdiff_t MACRO_VAR(i) = (testTopicStart(testTopicString), 0);        \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = (testTopicFinish(), 1))

void testSuiteStart(string mainTopic);
int testSuiteFinish();

void testTopicStart(string testTopic);
void testTopicFinish();

void unitTestStart(string testName);

void testSuccess();

void testFailure();
void appendTestFailureStart();
void appendTestFailureFinish();

#ifdef __cplusplus
}
#endif

#endif
