#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/array-types.h" // for flo_char_a
#include "util/text/string.h" // for flo_string
#include "util/types.h"       // for int64_t, uint64_t

#define FLO_NEWLINE 0x01

typedef struct {
    uint32_t *buffer;
    uint64_t size;
    uint32_t width;
    uint32_t height;
    uint32_t scanline;
} flo_ScreenDimension;
void flo_setupScreen(flo_ScreenDimension dimension);
void flo_printToScreen(flo_string data, uint8_t flags);

void flo_printToSerial(flo_string data, uint8_t flags);

flo_string flo_stringWithMinSize(flo_string data, unsigned char minSize,
                                 flo_char_a tmp);
flo_string flo_stringWithMinSizeDefault(flo_string data, unsigned char minSize);

flo_string flo_boolToString(bool data);

flo_string flo_ptrToString(void *data, flo_char_a tmp);
flo_string flo_ptrToStringDefault(void *data);

flo_string flo_charToString(char data, flo_char_a tmp);
flo_string flo_charToStringDefault(char data);

flo_string flo_stringToString(flo_string data);

flo_string flo_uint64ToString(uint64_t data, flo_char_a tmp);
flo_string flo_uint64ToStringDefault(uint64_t data);

flo_string flo_int64ToString(int64_t data, flo_char_a tmp);
flo_string flo_int64ToStringDefault(int64_t data);

flo_string flo_doubleToString(double data, flo_char_a tmp);
flo_string flo_doubleToStringDefault(double data);

flo_string flo_noAppend();

#define FLO_CONVERT_TO_STRING(data)                                            \
    _Generic((data),                                                           \
        flo_string: flo_stringToString,                                        \
        void *: flo_ptrToStringDefault,                                        \
        int *: flo_ptrToStringDefault,                                         \
        uint8_t *: flo_ptrToStringDefault,                                     \
        unsigned int *: flo_ptrToStringDefault,                                \
        char: flo_charToStringDefault,                                         \
        int64_t: flo_int64ToStringDefault,                                     \
        double: flo_doubleToStringDefault,                                     \
        uint64_t: flo_uint64ToStringDefault,                                   \
        uint32_t: flo_uint64ToStringDefault,                                   \
        uint16_t: flo_uint64ToStringDefault,                                   \
        uint8_t: flo_uint64ToStringDefault,                                    \
        int: flo_int64ToStringDefault,                                         \
        short: flo_int64ToStringDefault,                                       \
        bool: flo_boolToString,                                                \
        default: flo_noAppend)(data)

#define FLO_SERIAL_DATA(data, flags)                                           \
    flo_printToSerial(FLO_CONVERT_TO_STRING(data), flags)

#define FLO_SERIAL_1(data) FLO_SERIAL_DATA(data, 0)
#define FLO_SERIAL_2(data, flags) FLO_SERIAL_DATA(data, flags)

#define FLO_SERIAL_CHOOSER_IMPL_1(arg1) FLO_SERIAL_1(arg1)
#define FLO_SERIAL_CHOOSER_IMPL_2(arg1, arg2) FLO_SERIAL_2(arg1, arg2)
#define FLO_SERIAL_CHOOSER(...) FLO_SERIAL_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define FLO_SERIAL_CHOOSER_IMPL(_1, _2, N, ...) FLO_SERIAL_CHOOSER_IMPL_##N

#define FLO_SERIAL(...) FLO_SERIAL_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define FLO_LOG_DATA(data, flags)                                              \
    flo_printToScreen(FLO_CONVERT_TO_STRING(data), flags)

#define FLO_LOG_1(data) FLO_LOG_DATA(data, 0)
#define FLO_LOG_2(data, flags) FLO_LOG_DATA(data, flags)

#define FLO_LOG_CHOOSER_IMPL_1(arg1) FLO_LOG_1(arg1)
#define FLO_LOG_CHOOSER_IMPL_2(arg1, arg2) FLO_LOG_2(arg1, arg2)
#define FLO_LOG_CHOOSER(...) FLO_LOG_CHOOSER_IMPL(__VA_ARGS__, 2, 1)
#define FLO_LOG_CHOOSER_IMPL(_1, _2, N, ...) FLO_LOG_CHOOSER_IMPL_##N

#define FLO_LOG(...) FLO_LOG_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
