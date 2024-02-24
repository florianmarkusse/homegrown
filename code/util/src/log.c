#include "util/log.h"
#include "util/assert.h"        // for FLO_ASSERT
#include "util/memory/arena.h"  // for flo_alloc, flo_arena
#include "util/memory/macros.h" // for FLO_ALIGNOF, FLO_SIZEOF
#include "util/memory/memory.h" // for memcpy, memset

#define FLO_LOG_STD_BUFFER_LEN 1 << 10
#define FLO_STRING_CONVERTER_BUF_LEN 1 << 10

unsigned char stringConverterBuf[FLO_STRING_CONVERTER_BUF_LEN];
static flo_char_a stringConverterBuffer = (flo_char_a){
    .buf = stringConverterBuf, .len = FLO_STRING_CONVERTER_BUF_LEN};

extern unsigned char glyphStart[] asm("_binary_resources_font_psf_start");

#define HORIZONTAL_PADDING 0
#define PIXEL_MARGIN 20
#define BYTES_PER_PIXEL 4

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

static flo_ScreenDimension dim;
void flo_setupScreen(flo_ScreenDimension dimension) { dim = dimension; }

void flo_printToScreen(flo_string data, uint8_t flags) {
    FLO_ASSERT(dim.buffer != 0);

    static psf2_t *font = (psf2_t *)&glyphStart;
    uint64_t glyphsPerLine =
        (dim.width - PIXEL_MARGIN * 2) / (font->width + HORIZONTAL_PADDING);
    uint32_t glyphsOnLine = 0;
    int bytesPerLine = (font->width + 7) / 8;
    for (int64_t i = 0; i < data.len; i++) {
        unsigned char *glyph =
            (unsigned char *)&glyphStart + font->headersize +
            (data.buf[i] < font->numglyph ? data.buf[i] : 0) *
                font->bytesperglyph;
        uint32_t offset =
            (glyphsOnLine / glyphsPerLine) * (dim.scanline * font->height) +
            (glyphsOnLine % glyphsPerLine) *
                (font->width + HORIZONTAL_PADDING) * BYTES_PER_PIXEL;
        for (uint32_t y = 0; y < font->height; y++) {
            // TODO: use SIMD instructions?
            uint32_t line = offset;
            uint32_t mask = 1 << (font->width - 1);
            for (uint32_t x = 0; x < font->width; x++) {
                // NOLINTNEXTLINE
                *((uint32_t *)((uint64_t)dim.buffer +
                               (PIXEL_MARGIN * dim.scanline) +
                               (PIXEL_MARGIN * BYTES_PER_PIXEL) + line)) =
                    ((((uint32_t)*glyph) & (mask)) != 0) * 0xFFFFFF;

                mask >>= 1;
                line += BYTES_PER_PIXEL;
            }
            glyph += bytesPerLine;
            offset += dim.scanline;
        }
        glyphsOnLine++;
    }
}

void flo_printToSerial(flo_string data, uint8_t flags) {
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
            "movb $0x80, %%al;addb $2,%%dl;outb %%al, %%dx;" /* LCR set divisor
                                                                mode */
            "movb $1, %%al;subb $3,%%dl;outb %%al, %%dx;"    /* DLL divisor lo
                                                                115200 */
            "xorb %%al, %%al;incb %%dl;outb %%al, %%dx;"  /* DLH divisor hi */
            "incb %%dl;outb %%al, %%dx;"                  /* FCR fifo off */
            "movb $0x43, %%al;incb %%dl;outb %%al, %%dx;" /* LCR 8N1, break on
                                                           */
            "movb $0x8, %%al;incb %%dl;outb %%al, %%dx;"  /* MCR Aux out 2 */
            "xorb %%al, %%al;subb $4,%%dl;inb %%dx, %%al" /* clear
                                                             receiver/transmitter
                                                           */
            :
            : "a"(0x3f9)
            : "rdx");
    }

    for (int64_t i = 0; i < data.len; i++) {
        PUTC(data.buf[i]);
    }

    char newline = '\n';
    if (flags & FLO_NEWLINE) {
        PUTC(newline);
    }
}

uint32_t flo_appendToSimpleBuffer(flo_string data, flo_char_d_a *array,
                                  flo_arena *perm) {
    if (array->len + data.len > array->cap) {
        int64_t newCap = (array->len + data.len) * 2;
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

flo_string flo_ptrToString(void *data, flo_char_a tmp) {
    tmp.buf[0] = '0';
    tmp.buf[1] = 'x';

    int64_t counter = 2;
    uint64_t u = (uint64_t)data;
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

flo_string flo_int64ToString(int64_t data, flo_char_a tmp) {
    unsigned char *end = tmp.buf + tmp.len;
    unsigned char *beg = end;
    int64_t t = data > 0 ? -data : data;
    do {
        *--beg = '0' - (unsigned char)(t % 10);
    } while (t /= 10);
    if (data < 0) {
        *--beg = '-';
    }
    return FLO_STRING_PTRS(beg, end);
}

flo_string flo_int64ToStringDefault(int64_t data) {
    return flo_int64ToString(data, stringConverterBuffer);
}

flo_string flo_doubleToString(double data, flo_char_a tmp) {
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
