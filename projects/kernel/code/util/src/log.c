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
static uint32_t bytesPerLine;
static uint32_t glyphStartOffset;
static uint32_t glyphStartVerticalOffset;

// The window has the start, which is a certain logical line. This can be a very
// long line, so we specify which is the oldest part of the logical line still
// visible (in the window).
// Analogous for the end of the window.
typedef struct {
    uint32_t logicalLineStart;
    uint64_t leastRecentChar; // Inclusive
    uint32_t logicalLineEnd;
    uint64_t mostRecentChar; // Inclusive
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
            uint32_t glyphCountAfterTab =
                glyphsDrawn +
                terminal.screenLines[screenLineIndex].tabSizes[glyphsDrawn];

            while (glyphsDrawn < glyphCountAfterTab) {
                drawGlyph(' ', topRightGlyphOffset);
                topRightGlyphOffset += glyphs.width;
                glyphsDrawn++;
            }

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
        topRightGlyphOffset += dim.scanline;
        memset(
            &dim.backingBuffer[topRightGlyphOffset], 0,
            (glyphsPerLine - terminal.screenLines[screenLineIndex].glyphLen) *
                glyphs.width * BYTES_PER_PIXEL);
    }
}
//
// void rewind(uint32_t screenLines) {
//     // TODO: can add memmove optimization here. But need to take care what
//     // happens when in between flushes, the char buffer has looped in its
//     // entirety.
//
//     // Yes, you can scroll into the void here if the buffer is not filled
//     yet,
//     // who does that?
//     screenLines = MIN(
//         screenLines,
//         RING_MINUS(terminal.window.screenLineStart,
//                    RING_INCREMENT(terminal.logicalLineToWrite,
//                    MAX_SCROLLBACK_LINES), MAX_SCROLLBACK_LINES));
//
//     terminal.window.screenLineStart = RING_MINUS(
//         terminal.window.screenLineStart, screenLines, MAX_SCROLLBACK_LINES);
//     terminal.window.screenLineEnd = RING_MINUS(
//         terminal.window.screenLineEnd, screenLines, MAX_SCROLLBACK_LINES);
//
//     terminal.isTailing = false;
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
//
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

void toTail() {
    // TODO: can add memmove optimization here. But need to take care what
    // happens when in between flushes, the char buffer has looped in its
    // entirety.
    terminal.window.logicalLineEnd = terminal.logicalLineToWrite;
    terminal.window.mostRecentChar = terminal.charCount - 1;
    // TODO: do we actually need the other window variables?

    // TODO: this can be a massive number of chars, need to take the minimum
    // with a set number of total chars
    uint32_t currentScreenLinesInTerminal = 0;
    uint32_t logicalLineToConsider =
        RING_INCREMENT(terminal.window.logicalLineEnd, MAX_SCROLLBACK_LINES);
    while (currentScreenLinesInTerminal < glyphsPerColumn) {
        logicalLineToConsider =
            RING_DECREMENT(logicalLineToConsider, MAX_SCROLLBACK_LINES);
        currentScreenLinesInTerminal +=
            toScreenLines(terminal.logicalLines[logicalLineToConsider].charLen);
    }

    // TODO: maybe SIMD???
    uint32_t mostRecentScreenLine = MAX_VALUE(mostRecentScreenLine);
    for (uint32_t i = 0; i < glyphsPerColumn; i++) {
        uint32_t logicalLineIndex =
            RING_PLUS(logicalLineToConsider, i, MAX_SCROLLBACK_LINES);
        bool toNext = true;
        for (uint64_t j = 0;
             j < terminal.logicalLines[logicalLineIndex].charLen; j++) {
            uint64_t totalCharIndex =
                terminal.logicalLines[logicalLineIndex].start + j;
            unsigned char ch =
                terminal.buf[RING_RANGE(totalCharIndex, FILE_BUF_LEN)];

            if (toNext) {
                mostRecentScreenLine =
                    RING_INCREMENT(mostRecentScreenLine, MAX_GLYPSH_PER_COLUMN);
                terminal.screenLines[mostRecentScreenLine].start =
                    totalCharIndex;
                terminal.screenLines[mostRecentScreenLine].glyphLen = 0;
                toNext = false;
            }

            switch (ch) {
            case '\n': {
                toNext = true;
                break;
            }
            case '\t': {
                uint32_t beforeTabGlyphLen =
                    terminal.screenLines[mostRecentScreenLine].glyphLen;
                uint8_t additionalSpace =
                    (uint8_t)(((beforeTabGlyphLen + TAB_SIZE_IN_GLYPHS) &
                               (MAX_VALUE(additionalSpace) -
                                (TAB_SIZE_IN_GLYPHS - 1))) -
                              beforeTabGlyphLen);

                if (beforeTabGlyphLen + additionalSpace > glyphsPerLine) {
                    uint8_t extraSpaceThisLine =
                        (uint8_t)(glyphsPerLine - beforeTabGlyphLen);
                    terminal.screenLines[mostRecentScreenLine]
                        .tabSizes[terminal.screenLines[mostRecentScreenLine]
                                      .glyphLen] = extraSpaceThisLine;
                    terminal.screenLines[mostRecentScreenLine].glyphLen =
                        glyphsPerLine;

                    mostRecentScreenLine = RING_INCREMENT(
                        mostRecentScreenLine, MAX_GLYPSH_PER_COLUMN);
                    terminal.screenLines[mostRecentScreenLine].start =
                        totalCharIndex;
                    terminal.screenLines[mostRecentScreenLine].glyphLen =
                        additionalSpace - extraSpaceThisLine;
                    terminal.screenLines[mostRecentScreenLine].tabSizes[0] =
                        (uint8_t)terminal.screenLines[mostRecentScreenLine]
                            .glyphLen;
                } else {
                    terminal.screenLines[mostRecentScreenLine]
                        .tabSizes[terminal.screenLines[mostRecentScreenLine]
                                      .glyphLen] = additionalSpace;
                    terminal.screenLines[mostRecentScreenLine].glyphLen =
                        beforeTabGlyphLen + additionalSpace;

                    if (terminal.screenLines[mostRecentScreenLine].glyphLen >=
                        glyphsPerLine) {
                        toNext = true;
                    }
                }

                break;
            }
            default: {
                terminal.screenLines[mostRecentScreenLine].glyphLen++;
                if (terminal.screenLines[mostRecentScreenLine].glyphLen >=
                    glyphsPerLine) {
                    toNext = true;
                }
                break;
            }
            }
        }
    }

    for (uint16_t i = 0; i < glyphsPerColumn; i++) {
        drawLine(RING_MINUS(mostRecentScreenLine, glyphsPerColumn - 1 - i,
                            MAX_GLYPSH_PER_COLUMN),

                 i);
    }

    switchToScreenDisplay();
}

// void prowind(uint16_t screenLines) {
//     uint64_t totalScreenLines =
//         terminal.lines[terminal.window.logicalLineEnd].len;
//     uint16_t screenLinesProwound = 0;
//     while (screenLinesProwound < screenLines) {
//
//
//         screenLinesProwound++;
//     }
//     terminal.window.logicalLineStart;
//     terminal.window.screenLineStartIndex;
// }

// void flushToScreen(uint64_t charactersToFlush) {
//     // This assumes FILE_BUF_LEN is larger than the number of max glyphs on
//     // screen, duh
//     uint32_t newLines = 0;
//     for (uint64_t i = RING_MINUS(terminal.nextCharInBuf,
//                                  MIN(charactersToFlush, maxGlyphsOnScreen),
//                                  FILE_BUF_LEN);
//          i != terminal.nextCharInBuf; i = RING_INCREMENT(i, FILE_BUF_LEN)) {
//         if (CURRENT_LOGICAL_LINE->previousAdd.newLine) {
//             newLines = newLines + (newLines < glyphsPerColumn);
//             terminal.lineIndex =
//                 RING_INCREMENT(terminal.lineIndex, MAX_GLYPSH_PER_COLUMN);
//             CURRENT_LOGICAL_LINE->glyphCount = 0;
//             CURRENT_LOGICAL_LINE->charCount = 0;
//         }
//
//         for (uint64_t j = 0; j < LINE_AT(RING_DECREMENT(terminal.lineIndex,
//                                                         MAX_GLYPSH_PER_COLUMN))
//                                      ->previousAdd.additionalSpace;
//              j++) {
//             CURRENT_LOGICAL_LINE->chars[CURRENT_LOGICAL_LINE->glyphCount] = '
//             '; CURRENT_LOGICAL_LINE->glyphCount++;
//         }
//
//         CURRENT_LOGICAL_LINE->previousAdd = addCharToLineAsASCI(i,
//         CURRENT_LOGICAL_LINE);
//     }
//
//     memmove(&dim.backingBuffer[glyphStartVerticalOffset],
//             &dim.backingBuffer[glyphStartVerticalOffset +
//                                (dim.scanline * glyphs.height * newLines)],
//             dim.scanline * BYTES_PER_PIXEL * glyphs.height *
//                 (glyphsPerColumn - newLines));
//
//     // We are, always, at least redrawing the last line.
//     newLines = newLines + (newLines < glyphsPerColumn);
//     // Need to count backwards from terminal.lineIndex, the line that
//     contains
//         // the most recent content to the oldest line that contains new
//         content.
//         // Moreover, terminal.lineIndex already counts as a line, so we
//         decrease
//             // by 1.
//             uint64_t firstNewLineIndex =
//         RING_MINUS(terminal.lineIndex, newLines - 1, MAX_GLYPSH_PER_COLUMN);
//     for (uint32_t i = 0; i < newLines; i++) {
//         drawLine(terminal.indexes[RING_PLUS(firstNewLineIndex, i,
//                                             MAX_GLYPSH_PER_COLUMN)],
//                  (glyphsPerColumn - newLines) + i);
//     }
//
//     switchToScreenDisplay();
// }

// We are going to flush to:
// - The in-memory standin file buffer, this will be replaced by a file
// buffer in the future.
bool flushBuffer(uint8_max_a *buffer) {
    // TODO: flush buffer to file system here.

    // TODO: SIMD up in this birch.
    for (uint64_t i = 0; i < buffer->len; i++) {
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
