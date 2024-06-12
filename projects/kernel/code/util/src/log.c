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
typedef struct {
    uint8_t buf[FILE_BUF_LEN];
    uint64_t newCharIndex;
} FileBuffer;

static FileBuffer fileBuffer;

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

#define TAB_SIZE_IN_GLYPHS (1 << 2)

static ScreenDimension dim;
static bool isTailing = true;
static uint32_t glyphsPerLine;
static uint32_t glyphsPerColumn;
static uint32_t maxGlyphsOnScreen;
static uint32_t bytesPerLine;
static uint32_t glyphStartOffset;
static uint32_t glyphStartVerticalOffset;

#define MAX_GLYPSH_PER_LINE 512
typedef struct {
    uint8_t chars[MAX_GLYPSH_PER_LINE]; // TODO: should be glyphsperLine ;
    // Note that the glyphCount and charCount can be different values. A tab is
    // normally "worth" 4 glyphCounts but only 1 charCount.
    uint32_t glyphCount;
    uint32_t charCount;
    uint64_t charIndex; // The index in the file at which this line starts
} ScreenLine;

typedef struct {
    bool newLine;
    uint32_t startSpaces; // In case of "leftover" spaces with tabs
} CharAddResult;

#define MAX_GLYPSH_PER_COLUMN 256
typedef struct {
    uint32_t indexes[MAX_GLYPSH_PER_COLUMN];
    ScreenLine lines[MAX_GLYPSH_PER_COLUMN]; // TODO: should be glyphsperLine ;
    uint32_t lineIndex;
    CharAddResult previousAdd;
} Terminal;

static Terminal terminal;

void switchToScreenDisplay() {
    memcpy(dim.screen, dim.backingBuffer, dim.scanline * dim.height * 4);
}

void drawLine(ScreenLine *screenline, uint32_t rowNumber) {
    ASSERT(dim.backingBuffer != 0);

    psf2_t thing = glyphs;
    uint32_t verticalStart =
        glyphStartOffset + rowNumber * (dim.scanline * glyphs.height);
    for (uint8_t i = 0; i < screenline->glyphCount; i++) {
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
    uint32_t offset = verticalStart + screenline->glyphCount * (glyphs.width);
    for (uint32_t i = 0; i < glyphs.height; i++) {
        offset += dim.scanline;
        memset(&dim.backingBuffer[offset], 0,
               (glyphsPerLine - screenline->glyphCount) * glyphs.width * 4);
    }
}

void screenInit() {
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

void setupScreen(ScreenDimension dimension) {
    dim = dimension;
    dim.backingBuffer = graphicsBuffer;

    glyphsPerLine = (dim.width - HORIZONTAL_PIXEL_MARGIN * 2) / (glyphs.width);
    glyphsPerColumn =
        (dim.height - VERTICAL_PIXEL_MARGIN * 2) / (glyphs.height);
    maxGlyphsOnScreen = glyphsPerLine * glyphsPerColumn;
    glyphStartVerticalOffset = dim.scanline * VERTICAL_PIXEL_MARGIN;
    glyphStartOffset = glyphStartVerticalOffset + HORIZONTAL_PIXEL_MARGIN;
    bytesPerLine = (glyphs.width + 7) / 8;

    for (uint32_t i = 0; i < MAX_GLYPSH_PER_COLUMN; i++) {
        terminal.indexes[i] = i;
    }

    screenInit();
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

#define LINE_AT(_index) (terminal.lines[terminal.indexes[_index]])
#define CURRENT_LINE (terminal.lines[terminal.indexes[terminal.lineIndex]])
// Returns the lineIndex where the next char should be added.
CharAddResult addCharToLineAsASCI(uint64_t fileBufferIndex,
                                  uint32_t lineIndex) {
    unsigned char ch = fileBuffer.buf[fileBufferIndex];
    LINE_AT(lineIndex).charCount++;
    if (LINE_AT(lineIndex).charCount == 1) {
        LINE_AT(lineIndex).charIndex = fileBufferIndex;
    }

    switch (ch) {
    case '\t': {
        uint32_t previousCount = LINE_AT(lineIndex).glyphCount;
        uint32_t spacesToAdd =
            ((LINE_AT(lineIndex).glyphCount + TAB_SIZE_IN_GLYPHS) &
             (0xFFFFFFFF - (TAB_SIZE_IN_GLYPHS - 1))) -
            previousCount;
        for (uint32_t i = 0; i < spacesToAdd; i++) {
            LINE_AT(lineIndex).chars[LINE_AT(lineIndex).glyphCount] = ' ';
            LINE_AT(lineIndex).glyphCount++;

            if (LINE_AT(lineIndex).glyphCount >= glyphsPerLine) {
                return (CharAddResult){.startSpaces = spacesToAdd - i - 1,
                                       .newLine = true};
            }
        }
        break;
    }
    case '\n': {
        return (CharAddResult){.startSpaces = 0, .newLine = true};
    }
    default: {
        LINE_AT(lineIndex).chars[LINE_AT(lineIndex).glyphCount] = ch;
        LINE_AT(lineIndex).glyphCount++;

        if (LINE_AT(lineIndex).glyphCount >= glyphsPerLine) {
            return (CharAddResult){.startSpaces = 0, .newLine = true};
        }
    }
    }

    return (CharAddResult){.startSpaces = 0, .newLine = false};
}

void rewind(uint32_t lines) {
    uint64_t oldestLineIndex =
        increaseRingNonPowerOf2(terminal.lineIndex, 1, glyphsPerColumn);

    if (LINE_AT(oldestLineIndex).charIndex == fileBuffer.newCharIndex ||
        LINE_AT(oldestLineIndex).charCount == 0) {
        return;
    }

    lines = lines % glyphsPerColumn;
    // Note that we are going to process this many chars as this is the upper
    // bound of characters we can put in the lines we want to rewind. It is
    // likely that the first lines written will be overwritten by the relatively
    // newer characters that are read. This is intended behavior.
    uint64_t totalCharsToRewind = lines * glyphsPerLine;
    uint64_t firstIndexToRead =
        RING_MINUS(terminal.lines[terminal.indexes[oldestLineIndex]].charIndex,
                   totalCharsToRewind, FILE_BUF_LEN);

    uint32_t tempLineIndex = 0;
    // TODO: want to refactor this and pull out into function for sure.
    // Now lines 0 ... lines - 1 can be reused to write the rewound lines in.
    for (uint64_t i = firstIndexToRead; i < totalCharsToRewind; i++) {
        if (terminal.previousAdd.newLine) {
            tempLineIndex = (tempLineIndex + 1) % lines;
            LINE_AT(tempLineIndex).glyphCount = 0;
            LINE_AT(tempLineIndex).charCount = 0;
        }

        for (uint64_t j = 0; j < terminal.previousAdd.startSpaces; j++) {
            LINE_AT(tempLineIndex).chars[LINE_AT(tempLineIndex).glyphCount] =
                ' ';
            LINE_AT(tempLineIndex).glyphCount++;
        }

        terminal.previousAdd = addCharToLineAsASCI(i, terminal.lineIndex);
    }

    // Reorder oldest lines; the current content will be scrolled out of the
    // screen.

    memmove(&dim.backingBuffer[glyphStartVerticalOffset +
                               (dim.scanline * glyphs.height * lines)],
            &dim.backingBuffer[glyphStartVerticalOffset],
            dim.scanline * 4 * glyphs.height * (glyphsPerColumn - lines));

    for (uint32_t i = 0; i < lines; i++) {
        drawLine(&terminal.lines[terminal.indexes[i]], i);
    }

    switchToScreenDisplay();
}

void prowind(uint32_t lines) {
    if (LINE_AT(glyphsPerColumn).charCount == 0) {
        return;
    }
    //    lines = lines & (MAX_GLYPSH_PER_COLUMN - 1);
    //
    //    uint32_t lineToChangeIndex =
    //        (terminal.currentIndex - (glyphsPerColumn + 1)) &
    //        (MAX_GLYPSH_PER_COLUMN - 1);
    //    if (LINE_AT(lineToChangeIndex).charIndex == 0) {
    //        return;
    //    }
    //
    //    // otherwise, move 'lines' * glyphsPerLine chars forward and rewrite
    //    the
    //    // 'lines' number of lines in the array that now need to be rewritten.
    //    uint64_t firstNewCharIndex =
    //        (CURRENT_LINE.charIndex + CURRENT_LINE.charCount) & (FILE_BUF_LEN
    //        - 1);
    //    uint64_t afterLastPossibleIndex =
    //        (firstNewCharIndex + (lines * glyphsPerLine)) & (FILE_BUF_LEN -
    //        1);
    //
    //    for (uint64_t i = firstNewCharIndex; i != afterLastPossibleIndex;
    //         i = (i + 1) & (FILE_BUF_LEN - 1)) {
    //        LINE_AT(lineToChangeIndex)
    //    }
}

void flushToScreen(uint64_t charactersToFlush) {
    // This assumes FILE_BUF_LEN is larger than the number of max glyphs on
    // screen, duh
    uint32_t newLines = 0;
    for (uint64_t i = RING_MINUS(fileBuffer.newCharIndex,
                                 MIN(charactersToFlush, maxGlyphsOnScreen),
                                 FILE_BUF_LEN);
         i != fileBuffer.newCharIndex; i = RING_INCREMENT(i, FILE_BUF_LEN)) {
        if (terminal.previousAdd.newLine) {
            newLines = newLines + (newLines < glyphsPerColumn);
            terminal.lineIndex = (terminal.lineIndex + 1) % glyphsPerColumn;
            LINE_AT(terminal.lineIndex).glyphCount = 0;
            LINE_AT(terminal.lineIndex).charCount = 0;
        }

        for (uint64_t j = 0; j < terminal.previousAdd.startSpaces; j++) {
            LINE_AT(terminal.lineIndex)
                .chars[LINE_AT(terminal.lineIndex).glyphCount] = ' ';
            LINE_AT(terminal.lineIndex).glyphCount++;
        }

        terminal.previousAdd = addCharToLineAsASCI(i, terminal.lineIndex);
    }

    memmove(&dim.backingBuffer[glyphStartVerticalOffset],
            &dim.backingBuffer[glyphStartVerticalOffset +
                               (dim.scanline * glyphs.height * newLines)],
            dim.scanline * 4 * glyphs.height * (glyphsPerColumn - newLines));

    // We are, always, at least redrawing the last line.
    newLines = newLines + (newLines < glyphsPerColumn);
    // Need to count backwards from terminal.lineIndex, the line that contains
    // the most recent content to the oldest line that contains new content.
    // Moreover, terminal.lineIndex already counts as a line, so we decrease
    // by 1.
    uint64_t firstNewLineIndex = decreaseRingNonPowerOf2(
        terminal.lineIndex, (newLines - 1), glyphsPerColumn);
    for (uint32_t i = 0; i < newLines; i++) {
        drawLine(&terminal.lines[terminal.indexes[(firstNewLineIndex + i) %
                                                  glyphsPerColumn]],
                 (glyphsPerColumn - newLines) + i);
    }

    switchToScreenDisplay();
}

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(uint8_max_a *buffer) {
    // TODO: flush buffer to file system here.
    for (uint64_t bytesWritten = 0; bytesWritten < buffer->len;) {
        // the minimum of size remaining and what is left in the buffer.
        uint64_t spaceInBuffer = (FILE_BUF_LEN)-fileBuffer.newCharIndex;
        uint64_t dataToWrite = buffer->len - bytesWritten;
        uint64_t bytesToWrite = MIN(spaceInBuffer, dataToWrite);
        // TODO: this should become a file write pretty sure...
        memcpy(fileBuffer.buf + fileBuffer.newCharIndex,
               buffer->buf + bytesWritten, bytesToWrite);
        uint64_t before = fileBuffer.newCharIndex;
        fileBuffer.newCharIndex =
            RING_PLUS(fileBuffer.newCharIndex, bytesToWrite, FILE_BUF_LEN);
        bytesWritten += bytesToWrite;
    }

    if (isTailing) {
        flushToScreen(buffer->len);
    }

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
