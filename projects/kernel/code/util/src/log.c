#include "util/log.h"
#include "util/array-types.h"
#include "util/assert.h" // for ASSERT
#include "util/maths.h"
#include "util/memory/arena.h"  // for alloc, arena
#include "util/memory/macros.h" // for ALIGNOF, SIZEOF
#include "util/memory/memory.h" // for memcpy, memset

// TODO: replace with correct thing using memory allocators etc.
static uint32_t graphicsBuffer[1 << 20];

// TODO: Use triple mapped memory buffer to speed up ring buffer even more.
// TODO: Idea is to have a single flush buffer per thread and have it flush to
// the file buffer sometimes.
static uint8_t flushBuf000[128 * 64];
static uint8_max_a flushBuf = {.buf = flushBuf000, .cap = 128 * 64, .len = 0};

#define FILE_BUF_LEN (1ULL << 16LL)

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

#define BYTES_PER_PIXEL 4
#define VERTICAL_PIXEL_MARGIN 20
#define HORIZONTAL_PIXEL_MARGIN 20
#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

#define TAB_SIZE_IN_GLYPHS (1 << 2)

static ScreenDimension dim;
static uint16_t glyphsPerLine;
static uint16_t glyphsPerColumn;
static uint32_t maxGlyphsOnScreen;
static uint32_t maxCharsToProcess;
static uint32_t bytesPerLine;
static uint32_t glyphStartOffset;
static uint32_t glyphStartVerticalOffset;

typedef struct {
    uint32_t logicalLine;
    uint64_t charIndex;
} LineIndex;

// The window has the start, which is a certain logical line. This can be a very
// long line, so we specify which is the oldest part of the logical line still
// visible (in the window).
// Analogous for the end of the window.
typedef struct {
    LineIndex oldest;
    LineIndex newest;
} Window;

#define MAX_GLYPSH_PER_COLUMN (1 << 8)
#define MAX_GLYPSH_PER_LINE (1 << 8)
#define MAX_SCROLLBACK_LINES (1 << 16)

typedef struct {
    uint64_t start;
    uint64_t charLen;
} LogicalLine;

typedef struct {
    uint64_t start;
    uint32_t glyphLen;
    // This is the number of spaces the terminal has to draw when it encounters
    // a tab at this _glyphlen_ position.
    // TODO: can use a uint2_t here
    // since the domain is [1, 4]
    uint8_t tabSizes[MAX_GLYPSH_PER_LINE];
} ScreenLine;

typedef struct {
    LogicalLine logicalLines[MAX_SCROLLBACK_LINES];
    uint32_t logicalLineToWrite;
    ScreenLine
        screenLines[MAX_GLYPSH_PER_COLUMN]; // TODO: Use actual heap alloc
    uint32_t lastScreenLineIndex;
    bool newLine;
    bool isTailing;
    Window window;
    uint64_t charCount;
    uint64_t nextCharInBuf;
    uint8_t buf[FILE_BUF_LEN];
} Terminal;

static Terminal terminal = {.isTailing = true};

void switchToScreenDisplay() {
    memcpy(dim.screen, dim.backingBuffer,
           dim.scanline * dim.height * BYTES_PER_PIXEL);
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

    glyphsPerLine =
        (uint16_t)(dim.width - HORIZONTAL_PIXEL_MARGIN * 2) / (glyphs.width);
    glyphsPerColumn =
        (uint16_t)(dim.height - VERTICAL_PIXEL_MARGIN * 2) / (glyphs.height);
    maxGlyphsOnScreen = glyphsPerLine * glyphsPerColumn;
    maxCharsToProcess = 2 * maxGlyphsOnScreen;
    glyphStartVerticalOffset = dim.scanline * VERTICAL_PIXEL_MARGIN;
    glyphStartOffset = glyphStartVerticalOffset + HORIZONTAL_PIXEL_MARGIN;
    bytesPerLine = (glyphs.width + 7) / 8;

    screenInit();
}

bool flushStandardBuffer() { return flushBuffer(&flushBuf); }

#define CURRENT_LOGICAL_LINE                                                   \
    (terminal.logicalLines[terminal.logicalLineToWrite])

void drawGlyph(unsigned char ch, uint64_t topRightGlyphOffset) {
    unsigned char *glyph = &(glyphs.glyphs) + ch * glyphs.bytesperglyph;
    uint64_t glyphOffset = topRightGlyphOffset;
    for (uint32_t y = 0; y < glyphs.height; y++) {
        // TODO: use SIMD instructions?
        uint64_t line = glyphOffset;
        uint32_t mask = 1 << (glyphs.width - 1);
        for (uint32_t x = 0; x < glyphs.width; x++) {
            dim.backingBuffer[line] =
                ((((uint32_t)*glyph) & (mask)) != 0) * HAXOR_WHITE;
            mask >>= 1;
            line++;
        }
        glyph += bytesPerLine;
        glyphOffset += dim.scanline;
    }
}

void drawLine(uint32_t screenLineIndex, uint16_t rowNumber) {
    uint32_t topRightGlyphOffset =
        glyphStartOffset + rowNumber * (dim.scanline * glyphs.height);

    uint32_t glyphsDrawn = 0;
    uint64_t i =
        RING_RANGE(terminal.screenLines[screenLineIndex].start, FILE_BUF_LEN);
    while (glyphsDrawn < terminal.screenLines[screenLineIndex].glyphLen) {
        uint8_t ch = terminal.buf[i];

        switch (ch) {
        case '\t': {
            uint8_t spaceToAdd =
                terminal.screenLines[screenLineIndex].tabSizes[glyphsDrawn];
            for (uint32_t i = 0, glyphOffsetForSpaces = topRightGlyphOffset;
                 i < glyphs.height; i++) {
                memset(&dim.backingBuffer[glyphOffsetForSpaces], 0,
                       spaceToAdd * glyphs.width * BYTES_PER_PIXEL);
                glyphOffsetForSpaces += dim.scanline;
            }
            topRightGlyphOffset += glyphs.width * spaceToAdd;
            glyphsDrawn += spaceToAdd;

            break;
        }
        default: {
            drawGlyph(ch, topRightGlyphOffset);
            glyphsDrawn++;
            topRightGlyphOffset += glyphs.width;
            break;
        }
        }

        i = RING_INCREMENT(i, FILE_BUF_LEN);
    }

    // Zero/Black out the remaining part of the line.
    for (uint32_t i = 0; i < glyphs.height; i++) {
        memset(
            &dim.backingBuffer[topRightGlyphOffset], 0,
            (glyphsPerLine - terminal.screenLines[screenLineIndex].glyphLen) *
                glyphs.width * BYTES_PER_PIXEL);
        topRightGlyphOffset += dim.scanline;
    }
}

// void prowind(uint32_t screenLines) {
//     // TODO: can add memmove optimization here. But need to take care what
//     // happens when in between flushes, the char buffer has looped in its
//     // entirety.
//
//     screenLines = MIN(screenLines, RING_MINUS(terminal.logicalLineToWrite,
//                                               terminal.window.screenLineEnd,
//                                               MAX_SCROLLBACK_LINES));
//
//     terminal.window.screenLineStart = RING_PLUS(
//         terminal.window.screenLineStart, screenLines, MAX_SCROLLBACK_LINES);
//     terminal.window.screenLineEnd = RING_PLUS(
//         terminal.window.screenLineEnd, screenLines, MAX_SCROLLBACK_LINES);
//
//     terminal.isTailing = terminal.window.screenLineEnd ==
//     terminal.logicalLineToWrite;
//
//     for (uint16_t i = 0; i < glyphsPerColumn; i++) {
//         drawLine(
//             RING_PLUS(terminal.window.screenLineStart, i,
//             MAX_SCROLLBACK_LINES), i);
//     }
//
//     switchToScreenDisplay();
//
//     return;
// }

// A screenLine of 0 length still counts as a line.
uint64_t toScreenLines(uint64_t number) {
    return ((number > 0) * ((number + (glyphsPerLine - 1)) / glyphsPerLine)) +
           (number == 0);
}

LineIndex calculateOldest(LineIndex newest, uint32_t screenLinesSeen) {
    uint64_t charsInNewestLine =
        newest.charIndex - terminal.logicalLines[newest.logicalLine].start;
    uint64_t currentScreenLinesInTerminal = toScreenLines(charsInNewestLine);

    while (currentScreenLinesInTerminal < screenLinesSeen) {
        newest.logicalLine =
            RING_DECREMENT(newest.logicalLine, MAX_SCROLLBACK_LINES);
        currentScreenLinesInTerminal +=
            toScreenLines(terminal.logicalLines[newest.logicalLine].charLen);
    }

    uint64_t oldestCharLen = terminal.logicalLines[newest.logicalLine].charLen;
    if (oldestCharLen < maxCharsToProcess) {
        newest.charIndex = terminal.logicalLines[newest.logicalLine].start;
    } else {
        newest.charIndex = terminal.logicalLines[newest.logicalLine].start +
                           oldestCharLen - maxCharsToProcess;
    }

    return newest;
}

uint32_t processLogicalLine(LogicalLine logicalLine,
                            uint32_t currentScreenLine) {
    terminal.screenLines[currentScreenLine].start = logicalLine.start;
    terminal.screenLines[currentScreenLine].glyphLen = 0;

    // TODO: SIMD up in this bitch.
    bool toNext = false;
    uint64_t end = logicalLine.start + logicalLine.charLen;
    for (uint64_t charCount = logicalLine.start; charCount < end; charCount++) {
        unsigned char ch = terminal.buf[RING_RANGE(charCount, FILE_BUF_LEN)];

        if (toNext) {
            currentScreenLine =
                RING_INCREMENT(currentScreenLine, MAX_GLYPSH_PER_COLUMN);
            terminal.screenLines[currentScreenLine].start = charCount;
            terminal.screenLines[currentScreenLine].glyphLen = 0;
            toNext = false;
        }

        switch (ch) {
        case '\n': {
            return RING_INCREMENT(currentScreenLine, MAX_GLYPSH_PER_COLUMN);
        }
        case '\t': {
            uint32_t beforeTabGlyphLen =
                terminal.screenLines[currentScreenLine].glyphLen;
            uint8_t additionalSpace =
                (uint8_t)(((beforeTabGlyphLen + TAB_SIZE_IN_GLYPHS) &
                           (MAX_VALUE(additionalSpace) -
                            (TAB_SIZE_IN_GLYPHS - 1))) -
                          beforeTabGlyphLen);

            if (beforeTabGlyphLen + additionalSpace > glyphsPerLine) {
                uint8_t extraSpaceThisLine =
                    (uint8_t)(glyphsPerLine - beforeTabGlyphLen);
                terminal.screenLines[currentScreenLine]
                    .tabSizes[terminal.screenLines[currentScreenLine]
                                  .glyphLen] = extraSpaceThisLine;
                terminal.screenLines[currentScreenLine].glyphLen =
                    glyphsPerLine;

                currentScreenLine =
                    RING_INCREMENT(currentScreenLine, MAX_GLYPSH_PER_COLUMN);
                terminal.screenLines[currentScreenLine].start = charCount;
                terminal.screenLines[currentScreenLine].glyphLen =
                    additionalSpace - extraSpaceThisLine;
                terminal.screenLines[currentScreenLine].tabSizes[0] =
                    (uint8_t)terminal.screenLines[currentScreenLine].glyphLen;
            } else {
                terminal.screenLines[currentScreenLine]
                    .tabSizes[terminal.screenLines[currentScreenLine]
                                  .glyphLen] = additionalSpace;
                terminal.screenLines[currentScreenLine].glyphLen =
                    beforeTabGlyphLen + additionalSpace;

                if (terminal.screenLines[currentScreenLine].glyphLen >=
                    glyphsPerLine) {
                    toNext = true;
                }
            }

            break;
        }
        default: {
            terminal.screenLines[currentScreenLine].glyphLen++;
            if (terminal.screenLines[currentScreenLine].glyphLen >=
                glyphsPerLine) {
                toNext = true;
            }
            break;
        }
        }
    }

    return RING_INCREMENT(currentScreenLine, MAX_GLYPSH_PER_COLUMN);
}

// The last lines inside the window will always be drawn, the lines in the upper
// window may therefor not be shown in the end result.
void drawWindow() {
    // TODO: can add memmove optimization here. But need to take care what
    // happens when in between flushes, the char buffer has looped in its
    // entirety.
    uint32_t screenLineIndex = 0;
    if (terminal.window.oldest.logicalLine ==
        terminal.window.newest.logicalLine) {
        uint64_t totalChars = terminal.window.newest.charIndex -
                              terminal.window.oldest.charIndex + 1;
        screenLineIndex = processLogicalLine(
            (LogicalLine){.start = terminal.window.oldest.charIndex,
                          .charLen = totalChars},
            screenLineIndex);
    } else {
        // Process the first line, taking into account possible later start.
        uint64_t boundaryLineCharLen =
            terminal.logicalLines[terminal.window.oldest.logicalLine].charLen -
            (terminal.window.oldest.charIndex -
             terminal.logicalLines[terminal.window.oldest.logicalLine].start);
        screenLineIndex = processLogicalLine(
            (LogicalLine){.start = terminal.window.oldest.charIndex,
                          .charLen = boundaryLineCharLen},
            screenLineIndex);

        // Process lines in between the oldest and newest line.
        uint32_t inbetweenLogicalLineIndex = RING_INCREMENT(
            terminal.window.oldest.logicalLine, MAX_SCROLLBACK_LINES);
        while (inbetweenLogicalLineIndex !=
               terminal.window.newest.logicalLine) {
            screenLineIndex = processLogicalLine(
                terminal.logicalLines[inbetweenLogicalLineIndex],
                screenLineIndex);
            inbetweenLogicalLineIndex =
                RING_INCREMENT(inbetweenLogicalLineIndex, MAX_SCROLLBACK_LINES);
        }

        // Process the last line, taking into account possible earlier end.
        boundaryLineCharLen =
            terminal.window.newest.charIndex -
            terminal.logicalLines[terminal.window.newest.logicalLine].start + 1;
        screenLineIndex = processLogicalLine(
            (LogicalLine){
                .start =
                    terminal.logicalLines[terminal.window.newest.logicalLine]
                        .start,
                .charLen = boundaryLineCharLen},
            screenLineIndex);
    }

    terminal.lastScreenLineIndex =
        RING_DECREMENT(screenLineIndex, MAX_GLYPSH_PER_COLUMN);

    for (uint16_t i = 0; i < glyphsPerColumn; i++) {
        drawLine(RING_MINUS(screenLineIndex, glyphsPerColumn - i,
                            MAX_GLYPSH_PER_COLUMN),
                 i);
    }
}

void toTail() {
    terminal.window.newest.logicalLine = terminal.logicalLineToWrite;
    terminal.window.newest.charIndex = terminal.charCount - 1;
    terminal.window.oldest =
        calculateOldest(terminal.window.newest, glyphsPerColumn);

    drawWindow();

    switchToScreenDisplay();
}

void rewind(uint32_t screenLines) {
    // There are 2 paths here:
    //  - A precise rewind for all lines scrolled back <= glyphsPerColu,n
    //  - A less precise rewind for all lines scrolled back that are more than
    //  that.
    //  Remember to check for possibility to even rewind to this amount.

    if (screenLines <= glyphsPerLine) {
        uint64_t startOfFirstLine =
            terminal.screenLines[terminal.lastScreenLineIndex].start;

        terminal.window.newest.logicalLine = terminal.logicalLineToWrite;
        terminal.window.newest.charIndex = terminal.charCount - 1;
        terminal.window.oldest =
            calculateOldest(terminal.window.newest, glyphsPerColumn);

    } else {
    }

    terminal.window.newest.logicalLine = terminal.logicalLineToWrite;
    terminal.window.newest.charIndex = terminal.charCount - 1;
    terminal.window.oldest =
        calculateOldest(terminal.window.newest, glyphsPerColumn);

    switchToScreenDisplay();

    return;
}

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(uint8_max_a *buffer) {
    // TODO: flush buffer to file system here.

    uint64_t startIndex = 0;
    if (buffer->len > FILE_BUF_LEN) {
        startIndex = buffer->len - FILE_BUF_LEN;
    }

    // TODO: SIMD up in this birch.
    for (uint64_t i = startIndex; i < buffer->len; i++) {
        terminal.buf[terminal.nextCharInBuf] = buffer->buf[i];

        if (terminal.newLine) {
            terminal.logicalLineToWrite = RING_INCREMENT(
                terminal.logicalLineToWrite, MAX_SCROLLBACK_LINES);

            CURRENT_LOGICAL_LINE.start = terminal.charCount;
            CURRENT_LOGICAL_LINE.charLen = 0;
            terminal.newLine = false;
        }

        unsigned char ch = terminal.buf[terminal.nextCharInBuf];
        switch (ch) {
        case '\n': {
            CURRENT_LOGICAL_LINE.charLen++;
            terminal.newLine = true;
            break;
        }
        default: {
            CURRENT_LOGICAL_LINE.charLen++;
            break;
        }
        }

        terminal.nextCharInBuf =
            RING_INCREMENT(terminal.nextCharInBuf, FILE_BUF_LEN);
        terminal.charCount++;
    }

    if (terminal.isTailing) {
        toTail();
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
