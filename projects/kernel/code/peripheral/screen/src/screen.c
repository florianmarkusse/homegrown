#include "peripheral/screen/screen.h"

#include "cpu/x86.h"
#include "interoperation/array-types.h" // for U8_a, uint8_max_a, U8_d_a
#include "platform-abstraction/assert.h"      // for ASSERT
#include "interoperation/macros.h"
#include "interoperation/memory/definitions.h"
#include "memory/management/definitions.h"
#include "memory/management/virtual.h"
#include "memory/manipulation/manipulation.h"
#include "shared/maths/maths.h" // for RING_PLUS, RING_INCREMENT, RING_MINUS
#include "shared/memory/allocator/macros.h"

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
    U32 magic;
    U32 version;
    U32 headersize;
    U32 flags;
    U32 numglyph;
    U32 bytesperglyph;
    U32 height;
    U32 width;
    U8 glyphs[];
} __attribute__((packed)) psf2_t;

static U8 glyphsBinary[] = {
#embed "font.psf"
};
static psf2_t *font = (psf2_t *)&glyphsBinary;

static constexpr auto BYTES_PER_PIXEL = 4;
static constexpr auto VERTICAL_PIXEL_MARGIN = 20;
static constexpr auto HORIZONTAL_PIXEL_MARGIN = 20;
static constexpr auto HAXOR_GREEN = 0x0000FF00;
static constexpr auto HAXOR_WHITE = 0x00FFFFFF;

static constexpr auto TAB_SIZE_IN_GLYPHS = (1 << 2);

// NOTE: we write to this variable all the time, care should be taken when we
// move to multithreading
static ScreenDimension dim;

static U16 glyphsPerLine;
static U16 glyphsPerColumn;
static U32 maxGlyphsOnScreen;
static U32 maxCharsToProcess;
static U32 bytesPerLine;
static U32 glyphStartOffset;
static U32 glyphStartVerticalOffset;

static U16 ringGlyphsPerLine;
static U16 ringGlyphsPerColumn;

static constexpr auto MAX_SCROLLBACK_LINES = (1ULL << 13ULL);
static constexpr auto FILE_BUF_LEN = (1ULL << 16ULL);

static U64 *logicalLineLens;
static U64 logicalLines[MAX_SCROLLBACK_LINES];
static U32 logicalLineToWrite;
static bool logicalNewline;
static U64 *screenLines;
static U16 oldestScreenLineIndex;
static U64 *screenLinesCopy;

static bool lastScreenlineOpen;
static bool isTailing = true;
static U64 charCount;
static U32 nextCharInBuf;
static U8 *buf;

static void switchToScreenDisplay() {
    memcpy(dim.screen, dim.backingBuffer,
           dim.scanline * dim.height * BYTES_PER_PIXEL);
}

static void drawTerminalBox() {
    for (U32 y = 0; y < dim.height; y++) {
        for (U32 x = 0; x < dim.scanline; x++) {
            dim.backingBuffer[y * dim.scanline + x] = 0x00000000;
        }
    }

    for (U32 x = 0; x < dim.scanline; x++) {
        dim.backingBuffer[x] = HAXOR_GREEN;
    }
    for (U32 y = 0; y < dim.height; y++) {
        dim.backingBuffer[y * dim.scanline] = HAXOR_GREEN;
    }
    for (U32 y = 0; y < dim.height; y++) {
        dim.backingBuffer[y * dim.scanline + (dim.width - 1)] = HAXOR_GREEN;
    }

    for (U32 x = 0; x < dim.scanline; x++) {
        dim.backingBuffer[(dim.scanline * (dim.height - 1)) + x] = HAXOR_GREEN;
    }

    switchToScreenDisplay();
}

static void drawGlyph(U8 ch, U64 topRightGlyphOffset) {
    U8 *glyphStart = &(font->glyphs[ch * font->bytesperglyph]);
    U64 glyphOffset = topRightGlyphOffset;
    for (U32 y = 0; y < font->height; y++) {
        // TODO: use SIMD instructions?
        U64 line = glyphOffset;
        U32 mask = 1 << (font->width - 1);
        U8 glyphLine = *glyphStart;
        // NOTE: The important part is that the glyphLine captures the
        // whole of the line of the glyph, e.g., it covers one line of the glyph
        //
        // +----------+ +--+
        // 000001100000 0000
        // 000011110000 0000
        ASSERT(font->width <= sizeof(glyphLine));
        for (U32 x = 0; x < font->width; x++) {
            dim.backingBuffer[line] =
                (((glyphLine) & (mask)) != 0) * HAXOR_WHITE;
            mask >>= 1;
            line++;
        }
        glyphStart += bytesPerLine;
        glyphOffset += dim.scanline;
    }
}

static void zeroOutGlyphs(U32 topRightGlyphOffset, U16 numberOfGlyphs) {
    for (U32 i = 0; i < font->height; i++) {
        memset(&dim.backingBuffer[topRightGlyphOffset], 0,
               numberOfGlyphs * font->width * BYTES_PER_PIXEL);
        topRightGlyphOffset += dim.scanline;
    }
}

static void drawLines(U32 startIndex, U16 screenLinesToDraw,
                      U64 currentLogicalLineLen, U16 rowNumber) {
    U16 currentScreenLines = 0;

    U32 topRightGlyphOffset =
        glyphStartOffset + (rowNumber * (dim.scanline * font->height));
    U16 currentGlyphLen = 0;
    bool toNext = false;

    for (U64 i = screenLines[startIndex]; i < charCount; i++) {
        U8 ch = buf[RING_RANGE_VALUE(i, FILE_BUF_LEN)];

        if (toNext) {
            currentScreenLines++;
            if (currentScreenLines >= screenLinesToDraw) {
                break;
            }

            zeroOutGlyphs(topRightGlyphOffset, glyphsPerLine - currentGlyphLen);

            topRightGlyphOffset =
                glyphStartOffset + ((rowNumber + currentScreenLines) *
                                    (dim.scanline * font->height));
            currentGlyphLen = 0;
            toNext = false;
        }

        switch (ch) {
        case '\0': {
            break;
        }
        case '\n': {
            toNext = true;
            currentLogicalLineLen = 0;

            break;
        }
        case '\t': {
            U8 additionalSpace =
                (U8)(((currentLogicalLineLen + TAB_SIZE_IN_GLYPHS) &
                      (MAX_VALUE(additionalSpace) - (TAB_SIZE_IN_GLYPHS - 1))) -
                     currentLogicalLineLen);
            currentLogicalLineLen += additionalSpace;

            U16 finalSize = currentGlyphLen + additionalSpace;
            if (finalSize <= glyphsPerLine) {
                zeroOutGlyphs(topRightGlyphOffset, additionalSpace);
                topRightGlyphOffset += additionalSpace * font->width;
                currentGlyphLen = finalSize;
                toNext = (finalSize >= glyphsPerLine &&
                          buf[RING_RANGE_VALUE(i + 1, FILE_BUF_LEN)] != '\n');
            } else {
                // The tab overflows into the next screen line.
                U8 extraSpacePreviousLine =
                    (U8)(glyphsPerLine - currentGlyphLen);
                zeroOutGlyphs(topRightGlyphOffset, extraSpacePreviousLine);

                currentScreenLines++;
                if (currentScreenLines >= screenLinesToDraw) {
                    break;
                }

                topRightGlyphOffset =
                    glyphStartOffset + ((rowNumber + currentScreenLines) *
                                        (dim.scanline * font->height));
                currentGlyphLen = additionalSpace - extraSpacePreviousLine;

                zeroOutGlyphs(topRightGlyphOffset, currentGlyphLen);
                topRightGlyphOffset += currentGlyphLen * font->width;
            }

            break;
        }
        default: {
            drawGlyph(ch, topRightGlyphOffset);
            topRightGlyphOffset += font->width;
            currentGlyphLen++;
            currentLogicalLineLen++;
            if (currentGlyphLen >= glyphsPerLine &&
                buf[RING_RANGE_VALUE(i + 1, FILE_BUF_LEN)] != '\n') {
                toNext = true;
            }
            break;
        }
        }
    }

    zeroOutGlyphs(topRightGlyphOffset, glyphsPerLine - currentGlyphLen);
}

typedef struct {
    U16 realScreenLinesWritten;
    U16 currentScreenLineIndex;
    bool lastLineDone;
} FillResult;

static FillResult fillScreenLines(U64 dryStartIndex, U64 startIndex,
                                  U64 endIndexExclusive,
                                  U16 maxNewScreenLinesToFill) {
    U16 currentScreenLineIndex = 0;
    U16 screenLinesInWindow = dryStartIndex >= startIndex;

    U16 currentGlyphLen = 0;
    U64 currentLogicalLineLen = 0;

    bool toNext = false;
    // The parsing process is as follows:
    // 1. An index is marked as a new screenline
    // 2. The screenline is parsed.
    // TODO: SIMD up in this bitch.
    screenLinesCopy[currentScreenLineIndex] = dryStartIndex;
    logicalLineLens[currentScreenLineIndex] = currentLogicalLineLen;

    U64 i = dryStartIndex;
    for (; i < endIndexExclusive; i++) {
        U8 ch = buf[RING_RANGE_VALUE(i, FILE_BUF_LEN)];

        if (toNext) {
            if (maxNewScreenLinesToFill &&
                screenLinesInWindow >= maxNewScreenLinesToFill) {
                break;
            }

            currentScreenLineIndex =
                RING_INCREMENT(currentScreenLineIndex, ringGlyphsPerColumn);
            screenLinesCopy[currentScreenLineIndex] = i;
            logicalLineLens[currentScreenLineIndex] = currentLogicalLineLen;

            currentGlyphLen = 0;
            screenLinesInWindow += (i >= startIndex);

            toNext = false;
        }

        switch (ch) {
        case '\0': {
            break;
        }
        case '\n': {
            toNext = true;
            currentLogicalLineLen = 0;

            break;
        }
        case '\t': {
            U8 additionalSpace =
                (U8)(((currentLogicalLineLen + TAB_SIZE_IN_GLYPHS) &
                      (MAX_VALUE(additionalSpace) - (TAB_SIZE_IN_GLYPHS - 1))) -
                     currentLogicalLineLen);
            U16 finalSize = currentGlyphLen + additionalSpace;

            if (finalSize <= glyphsPerLine) {
                currentGlyphLen = finalSize;

                toNext = (finalSize == glyphsPerLine &&
                          buf[RING_RANGE_VALUE(i + 1, FILE_BUF_LEN)] != '\n');
            } else {
                // The tab overflows into the next screen line.
                // We mark the current screen line as having the
                // exclusive end index of the current tab. This
                // is fine as the drawLines function will black
                // out any remaining space in the screen line,
                // the same as a space.
                if (maxNewScreenLinesToFill &&
                    screenLinesInWindow >= maxNewScreenLinesToFill) {
                    break;
                }

                currentScreenLineIndex =
                    RING_INCREMENT(currentScreenLineIndex, ringGlyphsPerColumn);
                U8 extraSpacePreviousLine =
                    (U8)(glyphsPerLine - currentGlyphLen);
                // The tab is split up in 2 screen lines, so the
                // tab we encounter on the new line should know
                // that part of it is already drawn in the
                // previous line.
                currentGlyphLen = additionalSpace - extraSpacePreviousLine;
                screenLinesCopy[currentScreenLineIndex] = i;
                logicalLineLens[currentScreenLineIndex] =
                    currentLogicalLineLen + extraSpacePreviousLine;

                screenLinesInWindow += (i >= startIndex);
            }
            currentLogicalLineLen += additionalSpace;

            break;
        }
        default: {
            currentGlyphLen++;
            currentLogicalLineLen++;
            if (currentGlyphLen >= glyphsPerLine &&
                buf[RING_RANGE_VALUE(i + 1, FILE_BUF_LEN)] != '\n') {
                toNext = true;
            }
            break;
        }
        }
    }

    // Add entry indicating the end of the last actual screenline
    currentScreenLineIndex =
        RING_INCREMENT(currentScreenLineIndex, ringGlyphsPerColumn);
    screenLinesCopy[currentScreenLineIndex] = i;

    return (FillResult){.realScreenLinesWritten =
                            MIN(screenLinesInWindow, glyphsPerColumn),
                        .currentScreenLineIndex = currentScreenLineIndex,
                        .lastLineDone = toNext};
}

// A screenLine of 0 length still counts as a line.
// For the observent, all our screenlines have a length > 0 except when they are
// uninitialized, which is what the second operand of the addition handles
static U64 toScreenLines(U64 number) {
    return ((number > 0) * ((number + (glyphsPerLine - 1)) / glyphsPerLine)) +
           (number == 0);
}

static U32 I8IndexToLogicalLine(U64 I8Index) {
    if (I8Index >= logicalLines[logicalLineToWrite]) {
        return logicalLineToWrite;
    }

    U32 right = logicalLineToWrite;
    U32 left = RING_INCREMENT(right, MAX_SCROLLBACK_LINES);

    while (RING_MINUS(right, left, MAX_SCROLLBACK_LINES) > 1) {
        U32 mid =
            RING_PLUS(left, (RING_MINUS(right, left, MAX_SCROLLBACK_LINES) / 2),
                      MAX_SCROLLBACK_LINES);

        if (logicalLines[mid] > I8Index) {
            right = mid;
        } else {
            left = mid;
        }
    }

    return left;
}

static U64 oldestCharToParseGivenScreenLines(U64 endCharExclusive,
                                             U16 screenLinesToProcess) {
    U32 oldestLogicalLineIndex =
        RING_INCREMENT(logicalLineToWrite, MAX_SCROLLBACK_LINES);

    U32 logicalLineIndex = I8IndexToLogicalLine(endCharExclusive);
    if (logicalLineIndex == oldestLogicalLineIndex &&
        logicalLines[logicalLineIndex] >= endCharExclusive) {
        return endCharExclusive;
    }

    if (logicalLines[logicalLineIndex] == endCharExclusive) {
        logicalLineIndex =
            RING_DECREMENT(logicalLineIndex, MAX_SCROLLBACK_LINES);
    }

    U64 screenLinesProcessed =
        toScreenLines(endCharExclusive - logicalLines[logicalLineIndex]);

    while (screenLinesProcessed < screenLinesToProcess &&
           logicalLineIndex != oldestLogicalLineIndex) {
        screenLinesProcessed +=
            toScreenLines(logicalLines[logicalLineIndex] -
                          logicalLines[RING_DECREMENT(logicalLineIndex,
                                                      MAX_SCROLLBACK_LINES)]);

        logicalLineIndex =
            RING_DECREMENT(logicalLineIndex, MAX_SCROLLBACK_LINES);
    }

    U64 oldestCharToProcess = logicalLines[logicalLineIndex];

    if (endCharExclusive - oldestCharToProcess > maxCharsToProcess) {
        oldestCharToProcess = endCharExclusive - maxCharsToProcess;
    }

    return oldestCharToProcess;
}

static void toTail() {
    U16 finalScreenLineEntry =
        RING_PLUS(oldestScreenLineIndex, glyphsPerColumn - lastScreenlineOpen,
                  ringGlyphsPerColumn);
    U64 firstCharOutsideWindow = screenLines[finalScreenLineEntry];
    while (firstCharOutsideWindow <= screenLines[oldestScreenLineIndex] &&
           finalScreenLineEntry != oldestScreenLineIndex) {
        finalScreenLineEntry =
            RING_DECREMENT(finalScreenLineEntry, ringGlyphsPerColumn);
        firstCharOutsideWindow = screenLines[finalScreenLineEntry];
    }
    U64 oldestCharToProcess =
        MAX(oldestCharToParseGivenScreenLines(charCount, glyphsPerColumn),
            logicalLines[I8IndexToLogicalLine(firstCharOutsideWindow)]);

    FillResult fillResult = fillScreenLines(
        oldestCharToProcess, firstCharOutsideWindow, charCount, 0);

    U16 startIndex =
        RING_MINUS(fillResult.currentScreenLineIndex,
                   fillResult.realScreenLinesWritten, ringGlyphsPerColumn);

    U16 oldScreenLines = glyphsPerColumn - fillResult.realScreenLinesWritten;
    // You can write 10 new screen lines while your screen originally
    // displayed the first 40 screen lines, hence the distinction here.
    U16 originalScreenLines = RING_MINUS(
        finalScreenLineEntry, oldestScreenLineIndex, ringGlyphsPerColumn);

    U16 totalLines = fillResult.realScreenLinesWritten + originalScreenLines;
    if (totalLines > glyphsPerColumn) {
        oldestScreenLineIndex =
            RING_PLUS(oldestScreenLineIndex, totalLines - glyphsPerColumn,
                      ringGlyphsPerColumn);
    }

    for (U16 i = 0; i <= fillResult.realScreenLinesWritten; i++) {
        screenLines[RING_PLUS(oldestScreenLineIndex + oldScreenLines, i,
                              ringGlyphsPerColumn)] =
            screenLinesCopy[RING_PLUS(startIndex, i, ringGlyphsPerColumn)];
    }

    if (fillResult.realScreenLinesWritten < glyphsPerColumn) {
        U32 fromOffset =
            glyphStartVerticalOffset +
            (fillResult.realScreenLinesWritten - lastScreenlineOpen) *
                (dim.scanline * font->height);

        memmove(&dim.backingBuffer[glyphStartVerticalOffset],
                &dim.backingBuffer[fromOffset],
                oldScreenLines * (dim.scanline * font->height * 4));
    }

    U16 drawLineStartIndex =
        RING_PLUS(oldestScreenLineIndex, oldScreenLines, ringGlyphsPerColumn);
    drawLines(drawLineStartIndex, fillResult.realScreenLinesWritten,
              logicalLineLens[startIndex], oldScreenLines);

    lastScreenlineOpen = !fillResult.lastLineDone;

    switchToScreenDisplay();
}

bool flushToScreen(U8_max_a buffer) {
    U64 startIndex = 0;
    if (buffer.len > FILE_BUF_LEN) {
        startIndex = buffer.len - FILE_BUF_LEN;
    }

    // TODO: I think we can memcpy this all, and then
    // TODO: SIMD up in this birch.
    for (U64 i = startIndex; i < buffer.len; i++) {
        buf[nextCharInBuf] = buffer.buf[i];

        if (logicalNewline) {
            logicalLineToWrite =
                RING_INCREMENT(logicalLineToWrite, MAX_SCROLLBACK_LINES);

            logicalLines[logicalLineToWrite] = charCount;
            logicalNewline = false;
        }

        U8 ch = buf[nextCharInBuf];
        switch (ch) {
        case '\n': {
            logicalNewline = true;
            break;
        }
        default: {
            break;
        }
        }

        nextCharInBuf = RING_INCREMENT(nextCharInBuf, FILE_BUF_LEN);
        charCount++;
    }

    if (isTailing) {
        toTail();
    }

    return true;
}

void initScreen(ScreenDimension dimension, Arena *perm) {
    buf = NEW(perm, U8, FILE_BUF_LEN);
    // Need correct alignment
    U32 *doubleBuffer =
        NEW(perm, U32, CEILING_DIV_VALUE(dimension.size, (U32)BYTES_PER_PIXEL));

    dim = (ScreenDimension){.screen = dimension.screen,
                            .backingBuffer = doubleBuffer,
                            .size = dimension.size,
                            .width = dimension.width,
                            .height = dimension.height,
                            .scanline = dimension.scanline};

    PagedMemory pagedMemory = {.pageStart = (U64)dim.screen,
                               .numberOfPages =
                                   CEILING_DIV_EXP(dim.size, PAGE_FRAME_SHIFT)};
    mapVirtualRegionWithFlags((U64)dim.screen, pagedMemory, BASE_PAGE, PAT_3);
    flushCPUCaches();

    glyphsPerLine =
        (U16)(dim.width - HORIZONTAL_PIXEL_MARGIN * 2) / (font->width);
    glyphsPerColumn =
        (U16)(dim.height - VERTICAL_PIXEL_MARGIN * 2) / (font->height);

    ringGlyphsPerLine = (U16)next_pow2(glyphsPerLine);
    ringGlyphsPerColumn = (U16)next_pow2((glyphsPerColumn + 1));
    if (glyphsPerColumn == ringGlyphsPerColumn) {
        // Need to make screenLinesCopy a power of 2 larger because of
        // implementation.
        ringGlyphsPerColumn <<= 1;
    }
    screenLines = NEW(perm, U64, ringGlyphsPerColumn, ZERO_MEMORY);
    logicalLineLens = NEW(perm, U64, ringGlyphsPerColumn, ZERO_MEMORY);
    screenLinesCopy = NEW(perm, U64, ringGlyphsPerColumn, ZERO_MEMORY);

    maxGlyphsOnScreen = glyphsPerLine * glyphsPerColumn;
    maxCharsToProcess = 2 * maxGlyphsOnScreen;
    glyphStartVerticalOffset = dim.scanline * VERTICAL_PIXEL_MARGIN;
    glyphStartOffset = glyphStartVerticalOffset + HORIZONTAL_PIXEL_MARGIN;
    bytesPerLine = CEILING_DIV_VALUE(font->width, (U32)8);

    ASSERT(maxCharsToProcess <= FILE_BUF_LEN);

    drawTerminalBox();
}

static bool isWindowSmallerThanScreen() {
    return screenLines[oldestScreenLineIndex] >=
           screenLines[RING_PLUS(oldestScreenLineIndex, glyphsPerColumn,
                                 ringGlyphsPerColumn)];
}

void rewind(U16 numberOfScreenLines) {
    if (isWindowSmallerThanScreen()) {
        return;
    }

    U64 currentOldestCharInWindow = screenLines[oldestScreenLineIndex];
    U64 oldestCharToProcess = oldestCharToParseGivenScreenLines(
        currentOldestCharInWindow, numberOfScreenLines);

    if (oldestCharToProcess == currentOldestCharInWindow) {
        return;
    }

    FillResult fillResult = fillScreenLines(
        oldestCharToProcess, oldestCharToProcess, currentOldestCharInWindow, 0);

    U16 newScreenLinesOnTop =
        MIN(fillResult.realScreenLinesWritten, numberOfScreenLines);

    U16 startIndex = RING_MINUS(fillResult.currentScreenLineIndex,
                                newScreenLinesOnTop, ringGlyphsPerColumn);

    oldestScreenLineIndex = RING_MINUS(
        oldestScreenLineIndex, newScreenLinesOnTop, ringGlyphsPerColumn);

    for (U16 i = 0; i < newScreenLinesOnTop; i++) {
        screenLines[RING_PLUS(oldestScreenLineIndex, i, ringGlyphsPerColumn)] =
            screenLinesCopy[RING_PLUS(startIndex, i, ringGlyphsPerColumn)];
    }

    U32 fromOffset = glyphStartVerticalOffset +
                     newScreenLinesOnTop * (dim.scanline * font->height);

    memmove(&dim.backingBuffer[fromOffset],
            &dim.backingBuffer[glyphStartVerticalOffset],
            (glyphsPerColumn - newScreenLinesOnTop) *
                (dim.scanline * font->height * 4));

    drawLines(oldestScreenLineIndex, newScreenLinesOnTop,
              logicalLineLens[startIndex], 0);

    isTailing = false;
    lastScreenlineOpen = false;

    switchToScreenDisplay();
}

void prowind(U16 numberOfScreenLines) {
    if (isWindowSmallerThanScreen()) {
        return;
    }

    U64 firstCharOutsideWindow = screenLines[RING_PLUS(
        oldestScreenLineIndex, glyphsPerColumn, ringGlyphsPerColumn)];

    if (charCount == firstCharOutsideWindow) {
        return;
    }

    U64 oldestCharToProcess =
        oldestCharToParseGivenScreenLines(firstCharOutsideWindow, 0);

    FillResult fillResult =
        fillScreenLines(oldestCharToProcess, firstCharOutsideWindow, charCount,
                        numberOfScreenLines);
    U16 oldScreenLines = glyphsPerColumn - fillResult.realScreenLinesWritten;

    U16 startIndex =
        RING_MINUS(fillResult.currentScreenLineIndex,
                   fillResult.realScreenLinesWritten, ringGlyphsPerColumn);

    oldestScreenLineIndex =
        RING_PLUS(oldestScreenLineIndex, fillResult.realScreenLinesWritten,
                  ringGlyphsPerColumn);

    for (U16 i = 0; i <= fillResult.realScreenLinesWritten; i++) {
        screenLines[RING_PLUS(oldestScreenLineIndex + oldScreenLines, i,
                              ringGlyphsPerColumn)] =
            screenLinesCopy[RING_PLUS(startIndex, i, ringGlyphsPerColumn)];
    }

    U32 fromOffset =
        glyphStartVerticalOffset +
        fillResult.realScreenLinesWritten * (dim.scanline * font->height);

    memmove(&dim.backingBuffer[glyphStartVerticalOffset],
            &dim.backingBuffer[fromOffset],
            oldScreenLines * (dim.scanline * font->height * 4));

    U16 drawLineStartIndex =
        RING_PLUS(oldestScreenLineIndex, oldScreenLines, ringGlyphsPerColumn);
    drawLines(drawLineStartIndex, fillResult.realScreenLinesWritten,
              logicalLineLens[startIndex], oldScreenLines);

    isTailing = screenLines[RING_PLUS(oldestScreenLineIndex, glyphsPerColumn,
                                      ringGlyphsPerColumn)] == charCount;
    lastScreenlineOpen = !fillResult.lastLineDone;

    switchToScreenDisplay();
}
