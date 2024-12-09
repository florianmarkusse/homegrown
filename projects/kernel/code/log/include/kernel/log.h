#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

#include "shared/log.h"
#include "shared/macros.h"
#include "shared/text/converter.h"
#include "shared/text/string.h"
#include "shared/types/types.h"

void appendToFlushBuffer(string data, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

#define LOG_DATA(data, flags)                                                  \
    appendToFlushBuffer(CONVERT_TO_STRING(data), flags)

#define LOG_1(data) LOG_DATA(data, 0)
#define LOG_2(data, flags) LOG_DATA(data, flags)

#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define LOG_CHOOSER_IMPL(_1, _2, N, ...) LOG_CHOOSER_IMPL_##N

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define INFO(data, ...) LOG(data, ##__VA_ARGS__)

#define ERROR(data, ...) LOG(data, ##__VA_ARGS__)

#define FLUSH_AFTER_0                                                          \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#define FLUSH_AFTER_1(bufferType) FLUSH_AFTER_0

#define FLUSH_AFTER_IMPL(_0, _1, N, ...) FLUSH_AFTER_##N
#define FLUSH_AFTER(...)                                                       \
    FLUSH_AFTER_IMPL(__VA_ARGS__, 2, 1)                                        \
    (__VA_ARGS__)

#endif
