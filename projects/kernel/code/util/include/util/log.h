#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h" // for U32, U8, U64, I64, I8, U16
#include "text/converter.h"       // for U8_a, uint8_max_a
#include "text/string.h"          // for U8_a, uint8_max_a
#include "util/macros.h"          // for MACRO_VAR

#define NEWLINE 0x01
#define FLUSH 0x02

// This struct implicitly assumes that there are 4 bytes per pixel, hence a
// uint32 buffer
typedef struct {
    U32 *screen;
    U32 *backingBuffer;
    U64 size;
    U32 width;
    U32 height;
    U32 scanline;
} ScreenDimension;
void setupScreen(ScreenDimension dimension);
// TODO: needs buffer as argument when memory is set up
void appendToFlushBuffer(string data, U8 flags);
bool flushStandardBuffer();
bool flushBuffer(U8_max_a *buffer);

void rewind(U16 numberOfScreenLines);
void prowind(U16 numberOfScreenLines);

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

#ifdef __cplusplus
}
#endif

#endif
