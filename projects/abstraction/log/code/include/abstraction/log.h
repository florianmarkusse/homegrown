#ifndef ABSTRACTION_LOG_H
#define ABSTRACTION_LOG_H

#include "shared/macros.h"
#include "shared/text/converter.h"
#include "shared/text/string.h"
#include "shared/types/array-types.h"
#include "shared/types/types.h"

// NOTE: This is the basic logging implementation that all environments should
// implement if they want to do any sort of logging. Additionally, each
// environment is free to enhance their logging in any way they see fit.

U64 appendToBuffer(U8 *buffer, string data);
void appendToFlushBuffer(string data, U8 flags);
void appendZeroToFlushBuffer(U64 bytes, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

#define KLOG_APPEND(buffer, data)                                              \
    appendToBuffer(buffer, CONVERT_TO_STRING(data))

#define KLOG_DATA(data, flags)                                                 \
    appendToFlushBuffer(CONVERT_TO_STRING(data), flags)

#define KLOG_1(data) KLOG_DATA(data, 0)
#define KLOG_2(data, flags) KLOG_DATA(data, flags)

#define KLOG_CHOOSER_IMPL_1(arg1) KLOG_1(arg1)
#define KLOG_CHOOSER_IMPL_2(arg1, arg2) KLOG_2(arg1, arg2)
#define KLOG_CHOOSER_IMPL(_1, _2, N, ...) KLOG_CHOOSER_IMPL_##N
#define KLOG_CHOOSER(...) KLOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)

#define KLOG(...) KLOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) KLOG(data, ##__VA_ARGS__)

#define ERROR(data, ...) KLOG(data, ##__VA_ARGS__)

#define KFLUSH_AFTER                                                           \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#endif
