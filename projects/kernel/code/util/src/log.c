#include "util/log.h"
#include "util/array-types.h"
#include "util/assert.h" // for ASSERT
#include "util/maths.h"
#include "util/memory/arena.h"  // for alloc, arena
#include "util/memory/macros.h" // for ALIGNOF, SIZEOF
#include "util/memory/memory.h" // for memcpy, memset

static uint8_t flushBuf000[128 * 64];
static uint8_max_a flushBuf = {.buf = flushBuf000, .cap = 128 * 64, .len = 0};

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

extern psf2_t
    glyphs asm("_binary__home_florian_Desktop_homegrown_projects_kernel_"
               "code____resources_font_psf_start");

#define VERTICAL_PIXEL_MARGIN 20
#define HORIZONTAL_PIXEL_MARGIN 20
#define HAXOR_GREEN 0x0000FF00
#define HAXOR_WHITE 0x00FFFFFF

typedef struct {
    uint32_t x;
    uint32_t y;
} Cursor;

static ScreenDimension dim;
static uint32_t glyphsPerLine;
static uint32_t glyphsPerColumn;
static uint32_t bytesPerLine;
static uint32_t glyphStartOffset;
static uint32_t glyphStartVerticalOffset;

#define MAX_GLYPSH_PER_LINE 128
typedef struct {
    uint8_t chars[MAX_GLYPSH_PER_LINE]; // TODO: should be glyphsperLine ;
    uint8_t nextChar;
} ScreenLine;

#define MAX_GLYPSH_PER_COLUMN 64
typedef struct {
    ScreenLine lines[MAX_GLYPSH_PER_COLUMN]; // TODO: should be glyphsperLine ;
    int32_t lastFlushedLine;
    int32_t currentLine;
} ScreenBuffer;

static ScreenBuffer screenBuffer;

void drawLine(ScreenLine line, uint32_t rowNumber) {
    ASSERT(dim.buffer != 0);
    for (uint8_t i = 0; i < line.nextChar; i++) {
        uint8_t ch = line.chars[i];
        switch (ch) {
            // TODO: Special cases here:
            // \t and others?
        default: {
            unsigned char *glyph = &(glyphs.glyphs) + ch * glyphs.bytesperglyph;
            uint32_t offset = glyphStartOffset +
                              rowNumber * (dim.scanline * glyphs.height) +
                              i * (glyphs.width);

            for (uint32_t y = 0; y < glyphs.height; y++) {
                // TODO: use SIMD instructions?
                uint32_t line = offset;
                uint32_t mask = 1 << (glyphs.width - 1);
                for (uint32_t x = 0; x < glyphs.width; x++) {
                    dim.buffer[line] =
                        ((((uint32_t)*glyph) & (mask)) != 0) * HAXOR_WHITE;
                    mask >>= 1;
                    line++;
                }
                glyph += bytesPerLine;
                offset += dim.scanline;
            }
            break;
        }
        }
    }
}

// TODO: convert to using double buffer - you are reading from the graphics
// buffer directly which is not that fast + can cause artifacts.
void flushToScreen() {
    uint32_t distance =
        ABS(screenBuffer.lastFlushedLine - screenBuffer.currentLine);
    // The line that was last flushed needs to be included when writing new
    // lines because we cannot guarantee that it was fully written to on the
    // previous flush.
    uint32_t linesToRedraw = distance == 0 ? glyphsPerColumn : distance + 1;

    psf2_t test = glyphs;

    memmove(&dim.buffer[glyphStartVerticalOffset],
            &dim.buffer[glyphStartVerticalOffset +
                        (linesToRedraw - 1) * glyphs.height * dim.scanline],
            (glyphsPerColumn - linesToRedraw) * glyphs.height * dim.scanline *
                4);

    //    for (uint32_t i = linesToRedraw; i < glyphsPerColumn; i--) {
    //        memcpy(&(dim.buffer[glyphStartVerticalOffset +
    //                            (glyphsPerColumn - i - 1) * glyphs.height *
    //                                dim.scanline]),
    //               &(dim.buffer[glyphStartVerticalOffset +
    //                            (glyphsPerColumn - i + linesToRedraw - 2) *
    //                                glyphs.height * dim.scanline]),
    //               dim.scanline * glyphs.height);
    //    }

    for (uint32_t i = 0; i < linesToRedraw; i++) {
        drawLine(screenBuffer.lines[screenBuffer.lastFlushedLine + i],
                 glyphsPerColumn - linesToRedraw + i);
    }
    screenBuffer.lastFlushedLine = screenBuffer.currentLine;
}

void setupScreen(ScreenDimension dimension) {
    dim = dimension;
    glyphsPerLine = (dim.width - HORIZONTAL_PIXEL_MARGIN * 2) / (glyphs.width);
    glyphsPerColumn =
        (dim.height - VERTICAL_PIXEL_MARGIN * 2) / (glyphs.height);
    glyphStartVerticalOffset = dim.scanline * VERTICAL_PIXEL_MARGIN;
    glyphStartOffset = glyphStartVerticalOffset + HORIZONTAL_PIXEL_MARGIN;
    bytesPerLine = (glyphs.width + 7) / 8;

    for (uint32_t y = 0; y < dim.height; y++) {
        for (uint32_t x = 0; x < dim.scanline; x++) {
            dim.buffer[y * dim.scanline + x] = 0x00000000;
        }
    }

    for (uint32_t x = 0; x < dim.scanline; x++) {
        dim.buffer[x] = HAXOR_GREEN;
    }
    for (uint32_t y = 0; y < dim.height; y++) {
        dim.buffer[y * dim.scanline] = HAXOR_GREEN;
    }
    for (uint32_t y = 0; y < dim.height; y++) {
        dim.buffer[y * dim.scanline + (dim.width - 1)] = HAXOR_GREEN;
    }

    for (uint32_t x = 0; x < dim.scanline; x++) {
        dim.buffer[(dim.scanline * (dim.height - 1)) + x] = HAXOR_GREEN;
    }

    LOG(STRING("hi \nther"));
    LOG(glyphsPerLine);
    LOG(STRING(" "));
    LOG(glyphsPerColumn);
    LOG(STRING(" xgdhfgkjhfgiudhuir"));
    flushBuffer(&flushBuf);
    LOG(STRING("\n\n"));
    LOG(STRING("ghdkfhgjfkdh fhgbjkdf "));
    LOG(glyphsPerColumn, NEWLINE);
    LOG(STRING(" xgdhfgkjhfgiudhuir"));
    flushBuffer(&flushBuf);
}

void flushBuffer(uint8_max_a *buffer) {
    // TODO: flush buffer to file system here.

    for (uint64_t i = 0; i < buffer->len; i++) {
        unsigned char ch = buffer->buf[i];

        switch (ch) {
        // TODO: Add special cases for /t and /n
        case '\n': {
            screenBuffer.currentLine++;
            screenBuffer.lines[screenBuffer.currentLine].nextChar = 0;
            break;
        }
        default: {
            // TODO: SIMD this bad boy
            ScreenLine *currentLine =
                &screenBuffer.lines[screenBuffer.currentLine &
                                    (MAX_GLYPSH_PER_COLUMN - 1)];
            currentLine->chars[currentLine->nextChar] = ch;

            currentLine->nextChar++;

            if (currentLine->nextChar >= MAX_GLYPSH_PER_LINE) {
                screenBuffer.currentLine++;
                screenBuffer.lines[screenBuffer.currentLine].nextChar = 0;
            }

            break;
        }
        }
    }
    flushToScreen();

    buffer->len = 0;
}

// TODO: buffer should be a variable to this function once we have actual memory
// management set up instead of it being hardcoded.
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

// void printToScreen(string data, uint8_t flags) {
//     ASSERT(dim.buffer != 0);
//
//     for (int64_t i = 0; i < data.len; i++) {
//         uint8_t ch = data.buf[i];
//         switch (ch) {
//         case '\n': {
//             cursor.x = 0;
//             cursor.y++;
//             break;
//         }
//         default: {
//             unsigned char *glyph = &(glyphs.glyphs) + ch *
//             glyphs.bytesperglyph; uint32_t offset = dim.scanline *
//             PIXEL_MARGIN +
//                               cursor.y * (dim.scanline * glyphs.height) +
//                               PIXEL_MARGIN + cursor.x * (glyphs.width);
//
//             for (uint32_t y = 0; y < glyphs.height; y++) {
//                 // TODO: use SIMD instructions?
//                 uint32_t line = offset;
//                 uint32_t mask = 1 << (glyphs.width - 1);
//                 for (uint32_t x = 0; x < glyphs.width; x++) {
//                     dim.buffer[line] =
//                         ((((uint32_t)*glyph) & (mask)) != 0) * HAXOR_WHITE;
//                     mask >>= 1;
//                     line++;
//                 }
//                 glyph += bytesPerLine;
//                 offset += dim.scanline;
//             }
//
//             cursor.x = (cursor.x < glyphsPerLine) * (cursor.x + 1);
//             cursor.y += cursor.x == 0;
//             break;
//         }
//         }
//     }
//
//     if (flags & NEWLINE) {
//         cursor.x = 0;
//         cursor.y++;
//     }
// }

void printToSerial(string data, uint8_t flags) {
    static char serinit = 0;
#define PUTC(c)                                                                \
    __asm__ __volatile__(                                                      \
        "xorl %%ebx, %%ebx; movb %0, %%bl;"                                    \
        "movl $10000,%%ecx;"                                                   \
        "1:inb %%dx, %%al;pause;"                                              \
        "cmpb $0xff,%%al;je 2f;"                                               \
        "dec %%ecx;jz 2f;"                                                     \
        "andb $0x20,%%al;jz 1b;"                                               \
        "subb $5,%%dl;movb %%bl, %%al;outb %%al, %%dx;2:" ::"a"(c),            \
        "d"(0x3fd)                                                             \
        : "rbx", "rcx");
    /* initialize serial port */
    if (!serinit) {
        serinit = 1;
        __asm__ __volatile__(
            "movl %0, %%edx;"
            "xorb %%al, %%al;outb %%al, %%dx;"               /* IER int off */
            "movb $0x80, %%al;addb $2,%%dl;outb %%al, %%dx;" /* LCR set
                                                                divisor mode
                                                              */
            "movb $1, %%al;subb $3,%%dl;outb %%al, %%dx;"    /* DLL divisor lo
                                                                115200 */
            "xorb %%al, %%al;incb %%dl;outb %%al, %%dx;"     /* DLH divisor hi
                                                              */
            "incb %%dl;outb %%al, %%dx;"                     /* FCR fifo off */
            "movb $0x43, %%al;incb %%dl;outb %%al, %%dx;"    /* LCR 8N1, break
                                                              * on
                                                              */
            "movb $0x8, %%al;incb %%dl;outb %%al, %%dx;"     /* MCR Aux out 2 */
            "xorb %%al, %%al;subb $4,%%dl;inb %%dx, %%al"    /* clear
                                                                receiver/transmitter
                                                              */
            :
            : "a"(0x3f9)
            : "rdx");
    }

    for (uint64_t i = 0; i < data.len; i++) {
        PUTC(data.buf[i]);
    }

    char newline = '\n';
    if (flags & NEWLINE) {
        PUTC(newline);
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
