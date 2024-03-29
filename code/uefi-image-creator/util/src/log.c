#include "util/log.h"
#include "util/assert.h"        // for FLO_ASSERT
#include "util/maths.h"         // for FLO_MIN
#include "util/memory/macros.h" // for FLO_ALIGNOF, FLO_SIZEOF
#include <string.h>             // for memcpy, memset, strlen
#include <unistd.h>             // for isatty, write, STDERR_FILENO, STDOUT...

#define FLO_LOG_STD_BUFFER_LEN 1 << 10
#define FLO_STRING_CONVERTER_BUF_LEN 1 << 10

unsigned char stdoutBuf[FLO_LOG_STD_BUFFER_LEN];
unsigned char stderrBuf[FLO_LOG_STD_BUFFER_LEN];
unsigned char stringConverterBuf[FLO_STRING_CONVERTER_BUF_LEN];

static flo_WriteBuffer stdoutBuffer = (flo_WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = FLO_LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDOUT_FILENO};
static flo_WriteBuffer stderrBuffer = (flo_WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = FLO_LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDERR_FILENO};
static flo_char_a stringConverterBuffer = (flo_char_a){
    .buf = stringConverterBuf, .len = FLO_STRING_CONVERTER_BUF_LEN};

uint32_t flo_appendToSimpleBuffer(flo_string data, flo_char_d_a *array,
                                  flo_arena *perm) {
    if (array->len + data.len > array->cap) {
        ptrdiff_t newCap = (array->len + data.len) * 2;
        if (array->buf == NULL) {
            array->cap = data.len;
            array->buf = flo_alloc(perm, FLO_SIZEOF(unsigned char),
                                   FLO_ALIGNOF(unsigned char), newCap, 0);
        } else if (perm->end == (char *)(array->buf - array->cap)) {
            flo_alloc(perm, FLO_SIZEOF(unsigned char),
                      FLO_ALIGNOF(unsigned char), newCap, 0);
        } else {
            void *buf = flo_alloc(perm, FLO_SIZEOF(unsigned char),
                                  FLO_ALIGNOF(unsigned char), newCap, 0);
            memcpy(buf, array->buf, array->len);
            array->buf = buf;
        }

        array->cap = newCap;
    }
    memcpy(array->buf + array->len, data.buf, data.len);
    array->len += data.len;
    return (uint32_t)data.len;
}

bool flo_flushBuffer(flo_WriteBuffer *buffer) {
    for (ptrdiff_t bytesWritten = 0; bytesWritten < buffer->array.len;) {
        ptrdiff_t partialBytesWritten =
            write(buffer->fileDescriptor, buffer->array.buf + bytesWritten,
                  buffer->array.len - bytesWritten);
        if (partialBytesWritten < 0) {
            FLO_ASSERT(false);
            return false;
        } else {
            bytesWritten += partialBytesWritten;
        }
    }

    buffer->array.len = 0;

    return true;
}

flo_WriteBuffer *flo_getWriteBuffer(flo_BufferType bufferType) {
    if (bufferType == FLO_STDOUT) {
        return &stdoutBuffer;
    }
    return &stderrBuffer;
}

uint32_t flo_appendToFlushBuffer(flo_string data, flo_WriteBuffer *buffer,
                                 unsigned char flags) {
    for (ptrdiff_t bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        ptrdiff_t spaceInBuffer = (buffer->array.cap) - buffer->array.len;
        ptrdiff_t dataToWrite = data.len - bytesWritten;
        ptrdiff_t bytesToWrite = FLO_MIN(spaceInBuffer, dataToWrite);
        memcpy(buffer->array.buf + buffer->array.len, data.buf + bytesWritten,
               bytesToWrite);
        buffer->array.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            flo_flushBuffer(buffer);
        }
    }

    if (flags & FLO_NEWLINE) {
        if (buffer->array.len >= buffer->array.cap) {
            flo_flushBuffer(buffer);
        }
        buffer->array.buf[buffer->array.len] = '\n';
        buffer->array.len++;
    }

    if (flags & FLO_FLUSH) {
        flo_flushBuffer(buffer);
    }

    return (uint32_t)data.len;
}

static flo_string flo_ansiColorToCode[FLO_COLOR_NUMS] = {
    FLO_STRING("\x1b[31m"), FLO_STRING("\x1b[32m"), FLO_STRING("\x1b[33m"),
    FLO_STRING("\x1b[34m"), FLO_STRING("\x1b[35m"), FLO_STRING("\x1b[36m"),
    FLO_STRING("\x1b[0m"),
};

uint32_t flo_appendColor(flo_AnsiColor color, flo_BufferType bufferType) {
    flo_WriteBuffer *buffer = flo_getWriteBuffer(bufferType);
    return flo_appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                       ? flo_ansiColorToCode[color]
                                       : FLO_EMPTY_STRING,
                                   buffer, 0);
}
uint32_t flo_appendColorReset(flo_BufferType bufferType) {
    flo_WriteBuffer *buffer = flo_getWriteBuffer(bufferType);
    return flo_appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                       ? flo_ansiColorToCode[FLO_COLOR_RESET]
                                       : FLO_EMPTY_STRING,
                                   buffer, 0);
}

flo_string flo_charToString(char data, flo_char_a tmp) {
    tmp.buf[0] = data;
    return FLO_STRING_LEN(tmp.buf, 1);
}

flo_string flo_charToStringDefault(char data) {
    return flo_charToString(data, stringConverterBuffer);
}

flo_string flo_stringToString(flo_string data) { return data; }

flo_string flo_boolToString(bool data) {
    return (data ? FLO_STRING("true") : FLO_STRING("false"));
}

flo_string flo_cStrToString(char *data) {
    return FLO_STRING_LEN(data, strlen(data));
}

flo_string flo_ptrToString(void *data, flo_char_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    ptrdiff_t counter = 2;
    uintptr_t u = (uintptr_t)data;
    for (int i = 2 * sizeof(u) - 1; i >= 0; i--) {
        tmp.buf[counter++] = "0123456789abcdef"[(u >> (4 * i)) & 15];
    }

    return (flo_string){.len = counter - 1, .buf = tmp.buf};
}

flo_string flo_ptrToStringDefault(void *data) {
    return flo_ptrToString(data, stringConverterBuffer);
}

flo_string flo_uint64ToString(uint64_t data, flo_char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    do {
        *--beg = '0' + (unsigned char)(data % 10);
    } while (data /= 10);
    return (FLO_STRING_PTRS(beg, end));
}

flo_string flo_uint64ToStringDefault(uint64_t data) {
    return flo_uint64ToString(data, stringConverterBuffer);
}

flo_string flo_ptrdiffToString(ptrdiff_t data, flo_char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    ptrdiff_t t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (unsigned char)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return FLO_STRING_PTRS(beg, end);
}

flo_string flo_ptrdiffToStringDefault(ptrdiff_t data) {
    return flo_ptrdiffToString(data, stringConverterBuffer);
}

flo_string flo_doubleToString(double data, flo_char_a tmp) {
    ptrdiff_t tmpLen = 0;
    uint32_t prec = 1000000; // i.e. 6 decimals

    if (data < 0) {
        tmp.buf[tmpLen++] = '-';
        data = -data;
    }

    data += 0.5 / ((double)prec);      // round last decimal
    if (data >= (double)(-1UL >> 1)) { // out of long range?
        tmp.buf[tmpLen++] = 'i';
        tmp.buf[tmpLen++] = 'n';
        tmp.buf[tmpLen++] = 'f';
        return FLO_STRING_LEN(tmp.buf, tmpLen);
    }

    uint64_t integral = (uint64_t)data;
    uint64_t fractional = (uint64_t)((data - (double)integral) * (double)prec);

    unsigned char buf2[64];
    flo_char_a tmp2 = (flo_char_a){.buf = buf2, .len = 64};

    flo_string part = flo_uint64ToString(integral, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    tmp.buf[tmpLen++] = '.';

    unsigned char counter = 0;
    for (uint32_t i = prec / 10; i > 1; i /= 10) {
        if (i > fractional) {
            counter++;
        }
    }
    memset(tmp.buf + tmpLen, '0', counter);

    part = flo_uint64ToString(fractional, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    return FLO_STRING_LEN(tmp.buf, tmpLen);
}

flo_string flo_doubleToStringDefault(double data) {
    return flo_doubleToString(data, stringConverterBuffer);
}

flo_string flo_stringWithMinSize(flo_string data, unsigned char minSize,
                                 flo_char_a tmp) {
    if (data.len >= minSize) {
        return data;
    }

    memcpy(tmp.buf, data.buf, data.len);
    uint32_t extraSpace = (uint32_t)(minSize - data.len);
    memset(tmp.buf + data.len, ' ', extraSpace);

    return FLO_STRING_LEN(tmp.buf, data.len + extraSpace);
}

flo_string flo_stringWithMinSizeDefault(flo_string data,
                                        unsigned char minSize) {
    return flo_stringWithMinSize(data, minSize, stringConverterBuffer);
}

flo_string flo_noAppend() {
    FLO_ASSERT(false);
    return FLO_EMPTY_STRING;
}
