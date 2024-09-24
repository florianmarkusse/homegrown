#ifndef LOG_LOG_H
#define LOG_LOG_H

#include "interoperation/array-types.h"
#include "interoperation/types.h"
#include "memory/management/allocator/arena.h"
#include "text/converter.h"
#include "text/string.h"
#include "util/macros.h"

#define NEWLINE 0x01
#define FLUSH 0x02

void initLogger(Arena *perm);

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

#define FLUSH_AFTER                                                            \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#endif
