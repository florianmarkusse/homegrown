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
    uint64_t charLen;
    uint32_t logicalLineIndex;
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
    ScreenLine *screenLinePointers[MAX_GLYPSH_PER_COLUMN]; // TODO: Use actual
                                                           // heap alloc
    bool newLine;
    bool isTailing;
    uint16_t oldestScreenLineIndex;
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

    for (uint64_t i = 0; i < MAX_GLYPSH_PER_COLUMN; i++) {
        terminal.screenLinePointers[i] = &terminal.screenLines[i];
    }

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
    uint64_t i = RING_RANGE(terminal.screenLinePointers[screenLineIndex]->start,
                            FILE_BUF_LEN);
    while (glyphsDrawn <
           terminal.screenLinePointers[screenLineIndex]->glyphLen) {
        uint8_t ch = terminal.buf[i];

        switch (ch) {
        case '\t': {
            uint8_t spaceToAdd = terminal.screenLinePointers[screenLineIndex]
                                     ->tabSizes[glyphsDrawn];
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
        memset(&dim.backingBuffer[topRightGlyphOffset], 0,
               (glyphsPerLine -
                terminal.screenLinePointers[screenLineIndex]->glyphLen) *
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

// TODO: take into acoount overwriten lines.
LineIndex calculateOldestCharToProcess(LineIndex currentOldestChar,
                                       uint16_t screenLinesToProcess) {
    uint64_t charsIncurrentOldestCharLine =
        currentOldestChar.charIndex -
        terminal.logicalLines[currentOldestChar.logicalLine].start;
    uint64_t screenLinesProcessed = toScreenLines(charsIncurrentOldestCharLine);

    while (screenLinesProcessed < screenLinesToProcess) {
        uint32_t newLogicalLine =
            RING_DECREMENT(currentOldestChar.logicalLine, MAX_SCROLLBACK_LINES);
        if (terminal.logicalLines[newLogicalLine].start >
            currentOldestChar.charIndex) {
            currentOldestChar.charIndex =
                terminal.logicalLines[currentOldestChar.logicalLine].start;
            return currentOldestChar;
        }

        currentOldestChar.logicalLine = newLogicalLine;
        screenLinesProcessed += toScreenLines(
            terminal.logicalLines[currentOldestChar.logicalLine].charLen);
    }

    uint64_t oldestCharLen =
        terminal.logicalLines[currentOldestChar.logicalLine].charLen;
    if (oldestCharLen < maxCharsToProcess) {
        currentOldestChar.charIndex =
            terminal.logicalLines[currentOldestChar.logicalLine].start;
    } else {
        currentOldestChar.charIndex =
            terminal.logicalLines[currentOldestChar.logicalLine].start +
            oldestCharLen - maxCharsToProcess;
    }

    return currentOldestChar;
}

// Processes logical lines in a ring buffer [startingScreenLine,
// startingScreenLine + maxIndicesToWrite)
uint16_t processLogicalLine(LogicalLine logicalLine, uint16_t currentScreenLine,
                            uint32_t logicalLineIndex,
                            uint16_t startingScreenLine,
                            uint16_t maxIndicesToWrite) {
    ASSERT((maxIndicesToWrite & (maxIndicesToWrite - 1)) == 0);

    terminal.screenLinePointers[currentScreenLine]->start = logicalLine.start;
    terminal.screenLinePointers[currentScreenLine]->logicalLineIndex =
        logicalLineIndex;
    terminal.screenLinePointers[currentScreenLine]->charLen = 0;
    terminal.screenLinePointers[currentScreenLine]->glyphLen = 0;

    // TODO: SIMD up in this bitch.
    bool toNext = false;
    uint64_t end = logicalLine.start + logicalLine.charLen;
    for (uint64_t charCount = logicalLine.start; charCount < end; charCount++) {
        unsigned char ch = terminal.buf[RING_RANGE(charCount, FILE_BUF_LEN)];

        if (toNext) {
            currentScreenLine =
                startingScreenLine +
                RING_INCREMENT(currentScreenLine - startingScreenLine,
                               maxIndicesToWrite);
            terminal.screenLinePointers[currentScreenLine]->logicalLineIndex =
                logicalLineIndex;
            terminal.screenLinePointers[currentScreenLine]->charLen = 0;
            terminal.screenLinePointers[currentScreenLine]->glyphLen = 0;
            toNext = false;
        }

        terminal.screenLinePointers[currentScreenLine]->charLen++;

        switch (ch) {
        case '\n': {
            return currentScreenLine =
                       startingScreenLine +
                       RING_INCREMENT(currentScreenLine - startingScreenLine,
                                      maxIndicesToWrite);
        }
        case '\t': {
            uint32_t beforeTabGlyphLen =
                terminal.screenLinePointers[currentScreenLine]->glyphLen;
            uint8_t additionalSpace =
                (uint8_t)(((beforeTabGlyphLen + TAB_SIZE_IN_GLYPHS) &
                           (MAX_VALUE(additionalSpace) -
                            (TAB_SIZE_IN_GLYPHS - 1))) -
                          beforeTabGlyphLen);

            if (beforeTabGlyphLen + additionalSpace > glyphsPerLine) {
                uint8_t extraSpaceThisLine =
                    (uint8_t)(glyphsPerLine - beforeTabGlyphLen);
                terminal.screenLinePointers[currentScreenLine]
                    ->tabSizes[terminal.screenLinePointers[currentScreenLine]
                                   ->glyphLen] = extraSpaceThisLine;
                terminal.screenLinePointers[currentScreenLine]->glyphLen =
                    glyphsPerLine;

                currentScreenLine =
                    startingScreenLine +
                    RING_INCREMENT(currentScreenLine - startingScreenLine,
                                   maxIndicesToWrite);
                terminal.screenLinePointers[currentScreenLine]->start =
                    charCount;
                terminal.screenLinePointers[currentScreenLine]->glyphLen =
                    additionalSpace - extraSpaceThisLine;
                terminal.screenLinePointers[currentScreenLine]->tabSizes[0] =
                    (uint8_t)terminal.screenLinePointers[currentScreenLine]
                        ->glyphLen;
                terminal.screenLinePointers[currentScreenLine]->charLen = 1;
            } else {
                terminal.screenLinePointers[currentScreenLine]
                    ->tabSizes[terminal.screenLinePointers[currentScreenLine]
                                   ->glyphLen] = additionalSpace;
                terminal.screenLinePointers[currentScreenLine]->glyphLen =
                    beforeTabGlyphLen + additionalSpace;

                if (terminal.screenLinePointers[currentScreenLine]->glyphLen >=
                    glyphsPerLine) {
                    toNext = true;
                }
            }

            break;
        }
        default: {
            terminal.screenLinePointers[currentScreenLine]->glyphLen++;
            if (terminal.screenLinePointers[currentScreenLine]->glyphLen >=
                glyphsPerLine) {
                toNext = true;
            }
            break;
        }
        }
    }

    return currentScreenLine =
               startingScreenLine +
               RING_INCREMENT(currentScreenLine - startingScreenLine,
                              maxIndicesToWrite);
}

// The most recent lines inside the window will always be drawn, the lines in
// the upper window may therefor not be shown in the end result.
uint32_t fillScreenLines(Window window, uint16_t screenLineStartIndex,
                         uint16_t indicesToWrite) {
    // TODO: can add memmove optimization here. But need to take care what
    // happens when in between flushes, the char buffer has looped in its
    // entirety.
    uint16_t screenLineIndex = screenLineStartIndex;
    if (window.oldest.logicalLine == window.newest.logicalLine) {
        uint64_t totalChars =
            window.newest.charIndex - window.oldest.charIndex + 1;
        // TODO: fix these arguments!!!
        screenLineIndex =
            processLogicalLine((LogicalLine){.start = window.oldest.charIndex,
                                             .charLen = totalChars},
                               screenLineIndex, window.oldest.logicalLine,
                               screenLineStartIndex, indicesToWrite);
    } else {
        // Process the first line, taking into account possible later start.
        uint64_t boundaryLineCharLen =
            terminal.logicalLines[window.oldest.logicalLine].charLen -
            (window.oldest.charIndex -
             terminal.logicalLines[window.oldest.logicalLine].start);
        screenLineIndex =
            processLogicalLine((LogicalLine){.start = window.oldest.charIndex,
                                             .charLen = boundaryLineCharLen},
                               screenLineIndex, window.oldest.logicalLine,
                               screenLineStartIndex, indicesToWrite);

        // Process lines in between the oldest and newest line.
        uint32_t inbetweenLogicalLineIndex =
            RING_INCREMENT(window.oldest.logicalLine, MAX_SCROLLBACK_LINES);
        while (inbetweenLogicalLineIndex != window.newest.logicalLine) {
            screenLineIndex = processLogicalLine(
                terminal.logicalLines[inbetweenLogicalLineIndex],
                screenLineIndex, inbetweenLogicalLineIndex,
                screenLineStartIndex, indicesToWrite);
            inbetweenLogicalLineIndex =
                RING_INCREMENT(inbetweenLogicalLineIndex, MAX_SCROLLBACK_LINES);
        }

        // Process the last line, taking into account possible earlier end.
        boundaryLineCharLen =
            window.newest.charIndex -
            terminal.logicalLines[window.newest.logicalLine].start + 1;
        screenLineIndex = processLogicalLine(
            (LogicalLine){
                .start = terminal.logicalLines[window.newest.logicalLine].start,
                .charLen = boundaryLineCharLen},
            screenLineIndex, window.newest.logicalLine, screenLineStartIndex,
            indicesToWrite);
    }

    return screenLineIndex;
}

void toTail() {
    LineIndex newestCharToProcess =
        (LineIndex){.logicalLine = terminal.logicalLineToWrite,
                    .charIndex = terminal.charCount - 1};
    LineIndex oldestCharToProcess =
        calculateOldestCharToProcess(newestCharToProcess, glyphsPerColumn);

    uint32_t nextScreenLineIndex = fillScreenLines(
        (Window){.newest = newestCharToProcess, .oldest = oldestCharToProcess},
        0, MAX_GLYPSH_PER_COLUMN);
    terminal.oldestScreenLineIndex =
        RING_MINUS(nextScreenLineIndex, glyphsPerColumn, MAX_GLYPSH_PER_COLUMN);

    for (uint16_t i = 0; i < glyphsPerColumn; i++) {
        drawLine(
            RING_PLUS(terminal.oldestScreenLineIndex, i, MAX_GLYPSH_PER_COLUMN),
            i);
    }

    switchToScreenDisplay();
}

void rewind(uint16_t screenLines) {
    Window rewindWindow;

    rewindWindow.newest.logicalLine =
        terminal.screenLinePointers[terminal.oldestScreenLineIndex]
            ->logicalLineIndex;
    rewindWindow.newest.charIndex =
        terminal.screenLinePointers[terminal.oldestScreenLineIndex]->start;
    rewindWindow.newest.charIndex--;
    if (terminal.logicalLines[rewindWindow.newest.logicalLine].start >
        rewindWindow.newest.charIndex) {
        rewindWindow.newest.logicalLine = RING_DECREMENT(
            rewindWindow.newest.logicalLine, MAX_SCROLLBACK_LINES);
    }

    // Yes, it is possible for there to be characters to be rewound still that
    // are not yet overwritten. The logicalline however it was attached to in
    // this case, however, is being overwritten currently and I don't want to do
    // the archeology to find out if I can still rewind.
    if (terminal.logicalLines[rewindWindow.newest.logicalLine].start >
        rewindWindow.newest.charIndex) {
        return;
    }

    rewindWindow.oldest =
        calculateOldestCharToProcess(rewindWindow.newest, screenLines);

    // TODO: fix this, some bad boys in here are still pointing to the same
    // thing !!! Need to go in reverse because we are polluting the array as we
    // go.
    for (uint16_t i = glyphsPerColumn - screenLines - 1; i < MAX_VALUE(i);
         i--) {
        terminal.screenLinePointers[RING_PLUS(terminal.oldestScreenLineIndex,
                                              i + screenLines,
                                              MAX_GLYPSH_PER_COLUMN)] =
            terminal.screenLinePointers[RING_PLUS(
                terminal.oldestScreenLineIndex, i, MAX_GLYPSH_PER_COLUMN)];
    }

    fillScreenLines(rewindWindow, terminal.oldestScreenLineIndex, screenLines);

    for (uint16_t i = 0; i < glyphsPerColumn; i++) {
        drawLine(
            RING_PLUS(terminal.oldestScreenLineIndex, i, MAX_GLYPSH_PER_COLUMN),
            i);
    }

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
