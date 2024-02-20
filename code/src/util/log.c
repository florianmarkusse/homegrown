#include "util/log.h"
#include "types.h"

/**
 * Display (extremely minimal) formated message on serial
 */
void printf(char *fmt, ...) {
    __builtin_va_list args;
    int64_t arg;
    int len, sign, i;
    char *p, tmpstr[19], n;
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
    /* parse format and print */
    __builtin_va_start(args, fmt);
    arg = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '%')
                goto put;
            len = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                len *= 10;
                len += *fmt - '0';
                fmt++;
            }
            if (*fmt == 'd') {
                arg = __builtin_va_arg(args, int64_t);
                sign = 0;
                if ((int)arg < 0) {
                    arg = -arg;
                    sign++;
                }
                i = 18;
                tmpstr[i] = 0;
                do {
                    tmpstr[--i] = '0' + (arg % 10);
                    arg /= 10;
                } while (arg != 0 && i > 0);
                if (sign)
                    tmpstr[--i] = '-';
                if (len > 0 && len < 18) {
                    while (i > 18 - len)
                        tmpstr[--i] = ' ';
                }
                p = &tmpstr[i];
                goto putstring;
            } else if (*fmt == 'x') {
                arg = __builtin_va_arg(args, int64_t);
                i = 16;
                tmpstr[i] = 0;
                do {
                    n = arg & 0xf;
                    tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
                    arg >>= 4;
                } while (arg != 0 && i > 0);
                if (len > 0 && len <= 16) {
                    while (i > 16 - len)
                        tmpstr[--i] = '0';
                }
                p = &tmpstr[i];
                goto putstring;
            } else if (*fmt == 's') {
                p = __builtin_va_arg(args, char *);
            putstring:
                if (p == (void *)0)
                    p = "(null)";
                while (*p)
                    PUTC(*p++);
            }
        } else {
        put:
            PUTC(*fmt);
        }
        fmt++;
    }
    __builtin_va_end(args);
}
