#include "util/log.h"
#include "util/assert.h"        // for ASSERT
#include "util/maths.h"         // for MIN
#include "util/memory/macros.h" // for ALIGNOF, SIZEOF
#include <string.h>             // for memcpy, memset, strlen
#include <uchar.h>
#include <unistd.h> // for isatty, write, STDERR_FILENO, STDOUT...

#define LOG_STD_BUFFER_LEN 1 << 10
#define STRING_CONVERTER_BUF_LEN 1 << 10

unsigned char stdoutBuf[LOG_STD_BUFFER_LEN];
unsigned char stderrBuf[LOG_STD_BUFFER_LEN];
unsigned char stringConverterBuf[STRING_CONVERTER_BUF_LEN];

static WriteBuffer stdoutBuffer = (WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDOUT_FILENO};
static WriteBuffer stderrBuffer = (WriteBuffer){
    .array = {.buf = stdoutBuf, .cap = LOG_STD_BUFFER_LEN, .len = 0},
    .fileDescriptor = STDERR_FILENO};
static char_a stringConverterBuffer =
    (char_a){.buf = stringConverterBuf, .len = STRING_CONVERTER_BUF_LEN};

uint32_t appendToSimpleBuffer(string data, char_d_a *array, Arena *perm) {
    if (array->len + data.len > array->cap) {
        ptrdiff_t newCap = (array->len + data.len) * 2;
        if (array->buf == NULL) {
            array->cap = data.len;
            array->buf = alloc(perm, SIZEOF(unsigned char),
                               ALIGNOF(unsigned char), newCap, 0);
        } else if (perm->end == (char *)(array->buf - array->cap)) {
            alloc(perm, SIZEOF(unsigned char), ALIGNOF(unsigned char), newCap,
                  0);
        } else {
            void *buf = alloc(perm, SIZEOF(unsigned char),
                              ALIGNOF(unsigned char), newCap, 0);
            memcpy(buf, array->buf, array->len);
            array->buf = buf;
        }

        array->cap = newCap;
    }
    memcpy(array->buf + array->len, data.buf, data.len);
    array->len += data.len;
    return (uint32_t)data.len;
}

bool flushBuffer(WriteBuffer *buffer) {
    for (ptrdiff_t bytesWritten = 0; bytesWritten < buffer->array.len;) {
        ptrdiff_t partialBytesWritten =
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

WriteBuffer *getWriteBuffer(BufferType bufferType) {
    if (bufferType == STDOUT) {
        return &stdoutBuffer;
    }
    return &stderrBuffer;
}

uint32_t appendToFlushBuffer(string data, WriteBuffer *buffer,
                             unsigned char flags) {
    for (ptrdiff_t bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        ptrdiff_t spaceInBuffer = (buffer->array.cap) - buffer->array.len;
        ptrdiff_t dataToWrite = data.len - bytesWritten;
        ptrdiff_t bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memcpy(buffer->array.buf + buffer->array.len, data.buf + bytesWritten,
               bytesToWrite);
        buffer->array.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            flushBuffer(buffer);
        }
    }

    if (flags & NEWLINE) {
        if (buffer->array.len >= buffer->array.cap) {
            flushBuffer(buffer);
        }
        buffer->array.buf[buffer->array.len] = '\n';
        buffer->array.len++;
    }

    if (flags & FLUSH) {
        flushBuffer(buffer);
    }

    return (uint32_t)data.len;
}

static string ansiColorToCode[COLOR_NUMS] = {
    STRING("\x1b[31m"), STRING("\x1b[32m"), STRING("\x1b[33m"),
    STRING("\x1b[34m"), STRING("\x1b[35m"), STRING("\x1b[36m"),
    STRING("\x1b[0m"),
};

uint32_t appendColor(AnsiColor color, BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBuffer(
        isatty(buffer->fileDescriptor) ? ansiColorToCode[color] : EMPTY_STRING,
        buffer, 0);
}
uint32_t appendColorReset(BufferType bufferType) {
    WriteBuffer *buffer = getWriteBuffer(bufferType);
    return appendToFlushBuffer(isatty(buffer->fileDescriptor)
                                   ? ansiColorToCode[COLOR_RESET]
                                   : EMPTY_STRING,
                               buffer, 0);
}

string charToString(char data, char_a tmp) {
    tmp.buf[0] = data;
    return STRING_LEN(tmp.buf, 1);
}

string charToStringDefault(char data) {
    return charToString(data, stringConverterBuffer);
}

string stringToString(string data) { return data; }

string boolToString(bool data) {
    return (data ? STRING("true") : STRING("false"));
}

string cStrToString(char *data) { return STRING_LEN(data, strlen(data)); }

string ptrToString(void *data, char_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    ptrdiff_t counter = 2;
    uintptr_t u = (uintptr_t)data;
    for (int i = 2 * sizeof(u) - 1; i >= 0; i--) {
        tmp.buf[counter++] = "0123456789abcdef"[(u >> (4 * i)) & 15];
    }

    return (string){.len = counter - 1, .buf = tmp.buf};
}

string ptrToStringDefault(void *data) {
    return ptrToString(data, stringConverterBuffer);
}

string uint64ToString(uint64_t data, char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    do {
        *--beg = '0' + (unsigned char)(data % 10);
    } while (data /= 10);
    return (STRING_PTRS(beg, end));
}

string uint64ToStringDefault(uint64_t data) {
    return uint64ToString(data, stringConverterBuffer);
}

string ptrdiffToString(ptrdiff_t data, char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    ptrdiff_t t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (unsigned char)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return STRING_PTRS(beg, end);
}

string ptrdiffToStringDefault(ptrdiff_t data) {
    return ptrdiffToString(data, stringConverterBuffer);
}

string doubleToString(double data, char_a tmp) {
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
        return STRING_LEN(tmp.buf, tmpLen);
    }

    uint64_t integral = (uint64_t)data;
    uint64_t fractional = (uint64_t)((data - (double)integral) * (double)prec);

    unsigned char buf2[64];
    char_a tmp2 = (char_a){.buf = buf2, .len = 64};

    string part = uint64ToString(integral, tmp2);
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

    part = uint64ToString(fractional, tmp2);
    memcpy(tmp.buf + tmpLen, part.buf, part.len);
    tmpLen += part.len;

    return STRING_LEN(tmp.buf, tmpLen);
}

string doubleToStringDefault(double data) {
    return doubleToString(data, stringConverterBuffer);
}

string stringWithMinSize(string data, unsigned char minSize, char_a tmp) {
    if (data.len >= minSize) {
        return data;
    }

    memcpy(tmp.buf, data.buf, data.len);
    uint32_t extraSpace = (uint32_t)(minSize - data.len);
    memset(tmp.buf + data.len, ' ', extraSpace);

    return STRING_LEN(tmp.buf, data.len + extraSpace);
}

string stringWithMinSizeDefault(string data, unsigned char minSize) {
    return stringWithMinSize(data, minSize, stringConverterBuffer);
}

string noAppend() {
    ASSERT(false);
    return EMPTY_STRING;
}
