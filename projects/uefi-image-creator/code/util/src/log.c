#include "util/log.h"
#include "interoperation/assert.h" // for ASSERT
#include "interoperation/types.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/manipulation/manipulation.h" // for alignof, sizeof
#include "util/maths.h"                       // for FLO_MIN
#include "util/types.h"                       // for flo_U8_a, flo_U8_d_a
#include <unistd.h> // for isatty, write, STDERR_FILENO, STDOUT...

static constexpr auto FLO_LOG_STD_BUFFER_LEN = 1 << 10;
static constexpr auto STRING_CONVERTER_BUF_LEN = 1 << 10;

U8 stdoutBuf[FLO_LOG_STD_BUFFER_LEN];
U8 stderrBuf[FLO_LOG_STD_BUFFER_LEN];
U8 stringConverterBuf[STRING_CONVERTER_BUF_LEN];

static flo_WriteBuffer stdoutBuffer = (flo_WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = FLO_LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDOUT_FILENO};
static flo_WriteBuffer stderrBuffer = (flo_WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = FLO_LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDERR_FILENO};
static flo_U8_a stringConverterBuffer =
    (flo_U8_a){.buf = stringConverterBuf, .len = STRING_CONVERTER_BUF_LEN};

U32 flo_appendToSimpleBuffer(string data, flo_U8_d_a *array, Arena *perm) {
    if (array->len + data.len > array->cap) {
        U64 newCap = (array->len + data.len) * 2;
        if (array->buf == NULL) {
            array->cap = data.len;
            array->buf = alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
        } else if (perm->end == (U8 *)(array->buf - array->cap)) {
            alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
        } else {
            void *buf = alloc(perm, sizeof(U8), alignof(U8), newCap, 0);
            memcpy(buf, array->buf, array->len);
            array->buf = buf;
        }

        array->cap = newCap;
    }
    memcpy(array->buf + array->len, data.buf, data.len);
    array->len += data.len;
    return (U32)data.len;
}

bool flo_flushBuffer(flo_WriteBuffer *buffer) {
    for (U64 bytesWritten = 0; bytesWritten < buffer->array.len;) {
        U64 partialBytesWritten =
            write(buffer->fileDescriptor, buffer->array.buf + bytesWritten,
                  buffer->array.len - bytesWritten);
        if (partialBytesWritten < 0) {
            ASSERT(false);
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

U32 flo_appendToFlushBuffer(string data, flo_WriteBuffer *buffer, U8 flags) {
    for (U64 bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        U64 spaceInBuffer = (buffer->array.cap) - buffer->array.len;
        U64 dataToWrite = data.len - bytesWritten;
        U64 bytesToWrite = FLO_MIN(spaceInBuffer, dataToWrite);
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

    return (U32)data.len;
}

static string flo_ansiColorToCode[FLO_COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

U32 flo_appendColor(flo_AnsiColor color, flo_BufferType bufferType) {
    flo_WriteBuffer *buffer = flo_getWriteBuffer(bufferType);
    return flo_appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                       ? flo_ansiColorToCode[color]
                                       : EMPTY_STRING,
                                   buffer, 0);
}
U32 flo_appendColorReset(flo_BufferType bufferType) {
    flo_WriteBuffer *buffer = flo_getWriteBuffer(bufferType);
    return flo_appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                       ? flo_ansiColorToCode[FLO_COLOR_RESET]
                                       : EMPTY_STRING,
                                   buffer, 0);
}

string flo_U8ToString(U8 data, flo_U8_a tmp) {
    tmp.buf[0] = data;
    return STRING_LEN(tmp.buf, 1);
}

string flo_U8ToStringDefault(U8 data) {
    return flo_U8ToString(data, stringConverterBuffer);
}

string stringToString(string data) { return data; }

string flo_boolToString(bool data) {
    return (data ? STRING("true") : STRING("false"));
}

string flo_cStrToString(U8 *data) { return STRING_LEN(data, strlen(data)); }

string flo_ptrToString(void *data, flo_U8_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    U64 counter = 2;
    U64 u = (U64)data;
    for (int i = 2 * sizeof(u) - 1; i >= 0; i--) {
        tmp.buf[counter++] = "0123456789abcdef"[(u >> (4 * i)) & 15];
    }

    return (string){.len = counter - 1, .buf = tmp.buf};
}

string flo_ptrToStringDefault(void *data) {
    return flo_ptrToString(data, stringConverterBuffer);
}

string flo_uint64ToString(U64 data, flo_U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    do {
        *--beg = '0' + (U8)(data % 10);
    } while (data /= 10);
    return (STRING_PTRS(beg, end));
}

string flo_uint64ToStringDefault(U64 data) {
    return flo_uint64ToString(data, stringConverterBuffer);
}

string flo_ptrdiffToString(U64 data, flo_U8_a tmp) {
    U8 *end = tmp.buf + tmp.len;
    U8 *beg = end;
    U64 t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (U8)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return STRING_PTRS(beg, end);
}

string flo_ptrdiffToStringDefault(U64 data) {
    return flo_ptrdiffToString(data, stringConverterBuffer);
}

string flo_doubleToString(double data, flo_U8_a tmp) {
    U64 tmpLen = 0;
    U32 prec = 1000000; // i.e. 6 decimals

    if (data < 0) {
        tmp.buf[tmpLen++] = '-';
        data = -data;
    }

    data += 0.5 / ((double)prec);      // round last decimal
    if (data >= (double)(-1UL >> 1)) { // out of long range?
        tmp.buf[tmpLen++] = 'i';
        tmp.buf[tmpLen++] = 'n';
        tmp.buf[tmpLen++] = 'f';
        return STRING_LEN(tmp.buf, tmpLen);
    }

    U64 integral = (U64)data;
    U64 fractional = (U64)((data - (double)integral) * (double)prec);

    U8 buf2[64];
    flo_U8_a tmp2 = (flo_U8_a){.buf = buf2, .len = 64};

    string part = flo_uint64ToString(integral, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    tmp.buf[tmpLen++] = '.';

    U8 counter = 0;
    for (U32 i = prec / 10; i > 1; i /= 10) {
        if (i > fractional) {
            counter++;
        }
    }
    memset(tmp.buf + tmpLen, '0', counter);

    part = flo_uint64ToString(fractional, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    return STRING_LEN(tmp.buf, tmpLen);
}

string flo_doubleToStringDefault(double data) {
    return flo_doubleToString(data, stringConverterBuffer);
}

string stringWithMinSize(string data, U8 minSize, flo_U8_a tmp) {
    if (data.len >= minSize) {
        return data;
    }

    memcpy(tmp.buf, data.buf, data.len);
    U32 extraSpace = (U32)(minSize - data.len);
    memset(tmp.buf + data.len, ' ', extraSpace);

    return STRING_LEN(tmp.buf, data.len + extraSpace);
}

string stringWithMinSizeDefault(string data, U8 minSize) {
    return stringWithMinSize(data, minSize, stringConverterBuffer);
}

string flo_noAppend() {
    ASSERT(false);
    return EMPTY_STRING;
}
