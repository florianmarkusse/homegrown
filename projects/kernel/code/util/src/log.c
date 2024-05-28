#include "util/log.h"
#include "util/array-types.h"
#include "util/assert.h" // for ASSERT
#include "util/maths.h"
#include "util/memory/arena.h"  // for alloc, arena
#include "util/memory/macros.h" // for ALIGNOF, SIZEOF
#include "util/memory/memory.h" // for memcpy, memset

// TODO: replace with correct thing using memory allocators etc.
static uint32_t graphicsBuffer[1 << 20];

// TODO: Idea is to have a single flush buffer per thread and have it flush to
// the file buffer sometimes.
static uint8_t flushBuf000[128 * 64];
static uint8_max_a flushBuf = {.buf = flushBuf000, .cap = 128 * 64, .len = 0};

#define FILE_BUF_LEN (1ULL << 16LL)
static uint8_t standinFileBuf000[(FILE_BUF_LEN)];
// len is here more used as the index of the next char that it can write to. It
// is used as a ring buffer.
static uint8_max_a standinFileBuffer = {
    .buf = standinFileBuf000, .cap = (FILE_BUF_LEN), .len = 0};

// The header contains all the data for each glyph. After that comes numGlyph *
// bytesPerGlyph bytes.
//            padding
//  Font data    |
// +----------+ +--+
// 000001100000 0000
// 000011110000 0000
// 000110011000 0000
// 001100001100 0000
// 011000000110 0000
// 110000000011 0000
// 111111111111 0000
// 111111111111 0000
// 110000000011 0000
// 110000000011 0000
// 110000000011 0000
// 110000000011 0000
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;
} __attribute__((packed)) psf2_t;

extern psf2_t glyphs asm("_binary____resources_font_psf_start");

#define VERTICAL_PIXEL_MARGIN 20
#define HORIZONTAL_PIXEL_MARGIN 20
#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

static ScreenDimension dim;
static uint32_t glyphsPerLine;
static uint32_t glyphsPerColumn;
static uint32_t bytesPerLine;
static uint32_t glyphStartOffset;
static uint32_t glyphStartVerticalOffset;

#define MAX_GLYPSH_PER_LINE 128
typedef struct {
    uint8_t chars[MAX_GLYPSH_PER_LINE]; // TODO: should be glyphsperLine ;
    uint8_t len;
} ScreenLine;

#define MAX_GLYPSH_PER_COLUMN 64
typedef struct {
    ScreenLine lines[MAX_GLYPSH_PER_COLUMN]; // TODO: should be glyphsperLine ;
    uint64_t lastFlushedIndex;
} ScreenBuffer;
typedef struct {
    ScreenBuffer buffer_1;
    ScreenBuffer buffer_2;
    bool isFirst;
} Terminal;

static Terminal terminal;

void switchToScreenDisplay() {
    memcpy(dim.buffer, dim.backingBuffer, dim.scanline * dim.height * 4);
}

void drawLine(ScreenLine *screenline, uint32_t rowNumber) {
    ASSERT(dim.backingBuffer != 0);

    psf2_t thing = glyphs;
    uint32_t verticalStart =
        glyphStartOffset + rowNumber * (dim.scanline * glyphs.height);
    for (uint8_t i = 0; i < screenline->len; i++) {
        uint8_t ch = screenline->chars[i];
        // TODO: Special cases here:
        // \t and others?
        switch (ch) {
        case '\t': {
            ch = ' ';
            break;
        }
        default: {
            break;
        }
        }

        unsigned char *glyph = &(glyphs.glyphs) + ch * glyphs.bytesperglyph;
        uint32_t offset = verticalStart + i * (glyphs.width);

        for (uint32_t y = 0; y < glyphs.height; y++) {
            // TODO: use SIMD instructions?
            uint32_t line = offset;
            uint32_t mask = 1 << (glyphs.width - 1);
            for (uint32_t x = 0; x < glyphs.width; x++) {
                dim.backingBuffer[line] =
                    ((((uint32_t)*glyph) & (mask)) != 0) * HAXOR_WHITE;
                mask >>= 1;
                line++;
            }
            glyph += bytesPerLine;
            offset += dim.scanline;
        }
    }

    // Zero/Black out the remaining part of the line.
    uint32_t offset = verticalStart + screenline->len * (glyphs.width);
    for (uint32_t i = 0; i < glyphs.height; i++) {
        offset += dim.scanline;
        memset(&dim.backingBuffer[offset], 0,
               (glyphsPerLine - screenline->len) * glyphs.width * 4);
    }
}

void setupScreen(ScreenDimension dimension) {
    dim = dimension;
    dim.backingBuffer = graphicsBuffer;

    glyphsPerLine = (dim.width - HORIZONTAL_PIXEL_MARGIN * 2) / (glyphs.width);
    glyphsPerColumn =
        (dim.height - VERTICAL_PIXEL_MARGIN * 2) / (glyphs.height);
    glyphStartVerticalOffset = dim.scanline * VERTICAL_PIXEL_MARGIN;
    glyphStartOffset = glyphStartVerticalOffset + HORIZONTAL_PIXEL_MARGIN;
    bytesPerLine = (glyphs.width + 7) / 8;

    for (uint32_t y = 0; y < dim.height; y++) {
        for (uint32_t x = 0; x < dim.scanline; x++) {
            dim.backingBuffer[y * dim.scanline + x] = 0x00000000;
        }
    }

    for (uint32_t x = 0; x < dim.scanline; x++) {
        dim.backingBuffer[x] = HAXOR_GREEN;
    }
    for (uint32_t y = 0; y < dim.height; y++) {
        dim.backingBuffer[y * dim.scanline] = HAXOR_GREEN;
    }
    for (uint32_t y = 0; y < dim.height; y++) {
        dim.backingBuffer[y * dim.scanline + (dim.width - 1)] = HAXOR_GREEN;
    }

    for (uint32_t x = 0; x < dim.scanline; x++) {
        dim.backingBuffer[(dim.scanline * (dim.height - 1)) + x] = HAXOR_GREEN;
    }

    switchToScreenDisplay();
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

void memcpy_fromFileBuffer(uint8_t *start, uint64_t from,
                           uint8_t numberOfChars) {
    uint64_t toEndBuffer = FILE_BUF_LEN - from - 1;
    memcpy(start, &standinFileBuffer.buf[(from) & (FILE_BUF_LEN - 1)],
           MIN(toEndBuffer, numberOfChars));
    if (toEndBuffer < numberOfChars) {
        uint64_t fromStartBuffer = numberOfChars - toEndBuffer;
        memcpy(
            start,
            &standinFileBuffer.buf[(from + toEndBuffer) & (FILE_BUF_LEN - 1)],
            fromStartBuffer);
    }
}

void flushToScreen(uint64_t charactersToFlush) {
    // We use a duplicate array herer so we can reuse the lines that were not
    // touched in the case of a flush that did not add as many or more lines
    // than the number of lines we have in the graphics buffer.
    terminal.isFirst = !terminal.isFirst;

    ScreenLine *newLines =
        terminal.isFirst ? terminal.buffer_1.lines : terminal.buffer_2.lines;
    ScreenLine *cachedLines =
        terminal.isFirst ? terminal.buffer_2.lines : terminal.buffer_1.lines;

    // TODO: this can happen based on an interrupt I think, max every 40
    // milisceconds or smth, because I imagine we will be flushing more
    // often than that.
    uint64_t currentCursor = standinFileBuffer.len - 1;
    uint8_t glyphCount = 0;
    uint32_t currentLine = glyphsPerColumn;

    while (currentLine > 0 && charactersToFlush > 0) {
        unsigned char ch = standinFileBuffer.buf[currentCursor];

        switch (ch) {
        case '\n': {
            if (glyphCount == 0) {
                break;
            }

            memcpy_fromFileBuffer(&newLines[currentLine - 1].chars[0],
                                  (currentCursor + 1) & (FILE_BUF_LEN - 1),
                                  glyphCount);
            newLines[currentLine - 1].len = glyphCount;

            currentLine--;
            glyphCount = 0;
            break;
        }
            // This currently includes \t. We are reading the buffer from
            // front to back, so we don't know how to adjust to a tab character
            // :|
        default: {
            glyphCount++;

            if (glyphCount >= glyphsPerLine) {
                memcpy_fromFileBuffer(&newLines[currentLine - 1].chars[0],
                                      currentCursor, glyphCount);
                newLines[currentLine - 1].len = glyphCount;
                currentLine--;
                glyphCount = 0;
            }

            break;
        }
        }

        charactersToFlush--;
        currentCursor = (currentCursor - 1) & (FILE_BUF_LEN - 1);
    }

    ScreenLine *lastCachedLine = &cachedLines[glyphsPerColumn - 1];
    if (charactersToFlush == 0) {
        ScreenLine *mergedLine = &newLines[currentLine - 1];

        uint32_t cachedCharsToAdd =
            MIN(glyphsPerLine - glyphCount, lastCachedLine->len);
        uint32_t firstCachedCharToAdd = lastCachedLine->len - cachedCharsToAdd;
        memcpy(&mergedLine->chars[0],
               &lastCachedLine->chars[firstCachedCharToAdd], cachedCharsToAdd);

        mergedLine->len = (uint8_t)cachedCharsToAdd;
        // We still need to display all the cached chars that were not added to
        // the merged line.
        lastCachedLine->len -= cachedCharsToAdd;

        memcpy_fromFileBuffer(&mergedLine->chars[cachedCharsToAdd],
                              (currentCursor + 1) & (FILE_BUF_LEN - 1),
                              glyphCount);
        mergedLine->len += glyphCount;
    }

    uint32_t cachedLinesStartIndex =
        glyphsPerColumn - 1 - (lastCachedLine->len == 0);
    for (uint32_t i = 0; i < currentLine - 1; i++) {
        drawLine(&cachedLines[cachedLinesStartIndex - i], currentLine - 2 - i);
    }

    for (uint32_t i = currentLine - 1; i < glyphsPerColumn; i++) {
        drawLine(&newLines[i], i);
    }

    switchToScreenDisplay();
}

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file buffer
// in the future.
bool flushBuffer(uint8_max_a *buffer) {
    // TODO: flush buffer to file system here.
    // For now using a standby standinFileBuffer?
    for (uint64_t bytesWritten = 0; bytesWritten < buffer->len;) {
        // the minimum of size remaining and what is left in the buffer.
        uint64_t spaceInBuffer =
            (standinFileBuffer.cap) - standinFileBuffer.len;
        uint64_t dataToWrite = buffer->len - bytesWritten;
        uint64_t bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        // TODO: this should become a file write pretty sure...
        memcpy(standinFileBuffer.buf + standinFileBuffer.len,
               buffer->buf + bytesWritten, bytesToWrite);
        standinFileBuffer.len =
            (standinFileBuffer.len + bytesToWrite) & (FILE_BUF_LEN - 1);
        bytesWritten += bytesToWrite;
    }

    flushToScreen(buffer->len);

    buffer->len = 0;

    return true;
}

// TODO: buffer should be a variable to this function once we have actual
// memory management set up instead of it being hardcoded.
void appendToFlushBuffer(string data, unsigned char flags) {
    for (uint64_t bytesWritten = 0; bytesWritten < data.len;) {
        // the minimum of size remaining and what is left in the buffer.
        uint64_t spaceInBuffer = (flushBuf.cap) - flushBuf.len;
        uint64_t dataToWrite = data.len - bytesWritten;
        uint64_t bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        memcpy(flushBuf.buf + flushBuf.len, data.buf + bytesWritten,
               bytesToWrite);
        flushBuf.len += bytesToWrite;
        bytesWritten += bytesToWrite;
        if (bytesWritten < data.len) {
            flushBuffer(&flushBuf);
        }
    }

    if (flags & NEWLINE) {
        if (flushBuf.len >= flushBuf.cap) {
            flushBuffer(&flushBuf);
        }
        flushBuf.buf[flushBuf.len] = '\n';
        flushBuf.len++;
    }

    if (flags & FLUSH) {
        flushBuffer(&flushBuf);
    }
}

uint32_t appendToSimpleBuffer(string data, char_d_a *array, arena *perm) {
    if (array->len + data.len > array->cap) {
        int64_t newCap = (array->len + data.len) * 2;
        if (array->buf == NULL) {
            array->cap = data.len;
            array->buf = alloc(perm, SIZEOF(unsigned char),
                               ALIGNOF(unsigned char), newCap, 0);
        } else if (perm->end == (uint8_t *)(array->buf - array->cap)) {
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

#define STRING_CONVERTER_BUF_LEN 1 << 10
unsigned char stringConverterBuf[STRING_CONVERTER_BUF_LEN];
static char_a stringConverterBuffer =
    (char_a){.buf = stringConverterBuf, .len = STRING_CONVERTER_BUF_LEN};

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

string ptrToString(void *data, char_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    int64_t counter = 2;
    uint64_t u = (uint64_t)data;
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

string int64ToString(int64_t data, char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    int64_t t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (unsigned char)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return STRING_PTRS(beg, end);
}

string int64ToStringDefault(int64_t data) {
    return int64ToString(data, stringConverterBuffer);
}

string doubleToString(double data, char_a tmp) {
    int64_t tmpLen = 0;
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
