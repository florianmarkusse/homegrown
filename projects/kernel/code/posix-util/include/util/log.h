#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "memory/management/allocator/arena.h"
#include "text/converter.h" // for U8_a, uint8_max_a
#include "text/string.h"    // for U8_a, uint8_max_a
#include "util/macros.h"    // for MACRO_VAR
#include "util/types.h"     // for char_a, U8_d_a
#include <stdbool.h>        // for false, true, bool
#include <stddef.h>         // for ptrdiff_t
#include <stdint.h>         // for uint32_t, uint64_t

#define NEWLINE 0x01
#define FLUSH 0x02

bool flushBuffer();
void appendToFlushBuffer(string data, U8 flags);

#define LOG_DATA(data, flags)                                                  \
    appendToFlushBuffer(CONVERT_TO_STRING(data), flags)

#define LOG_1(data) LOG_DATA(data, 0)
#define LOG_2(data, flags) LOG_DATA(data, flags)

#define LOG_DATA_BUFFER_TYPE(data, bufferType, flags)                          \
    LOG_DATA_3(data, getWriteBuffer(bufferType), flags)

#define LOG_CHOOSER_IMPL_1(arg1) LOG_1(arg1)
#define LOG_CHOOSER_IMPL_2(arg1, arg2) LOG_2(arg1, arg2)
#define LOG_CHOOSER(...) LOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define LOG_CHOOSER_IMPL(_1, _2, N, ...) LOG_CHOOSER_IMPL_##N

#define LOG(...) LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define FLUSH_AFTER                                                            \
    for (U64 MACRO_VAR(i) = 0; MACRO_VAR(i) < 1;                               \
         MACRO_VAR(i) = flushStandardBuffer())

#ifdef __cplusplus
}
#endif

#endif
