#include "printing.h"
#include "efi/c-efi-protocol-simple-text-input.h"
#include "efi/c-efi-protocol-simple-text-output.h"
#include "efi/c-efi-system.h"
#include "globals.h"

void error(U16 *string) {
    globals.st->con_out->output_string(globals.st->con_out, string);
    CEfiInputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           C_EFI_SUCCESS) {
        ;
    }
    globals.st->runtime_services->reset_system(C_EFI_RESET_SHUTDOWN,
                                               C_EFI_SUCCESS, 0, NULL);
}

static const U16 *digits = u"0123456789ABCDEF";
void printNumber(USize number, U8 base) {
    U16 buffer[24]; // Hopefully enough for UINTN_MAX (UINT64_MAX) + sign
                           // character
    USize i = 0;
    bool negative = false;

    if (base > 16) {
        error(u"Invalid base specified!\r\n");
    }

    do {
        buffer[i++] = digits[number % base];
        number /= base;
    } while (number > 0);

    switch (base) {
    case 2:
        // Binary
        buffer[i++] = u'b';
        buffer[i++] = u'0';
        break;

    case 8:
        // Octal
        buffer[i++] = u'o';
        buffer[i++] = u'0';
        break;

    case 10:
        // Decimal
        if (negative)
            buffer[i++] = u'-';
        break;

    case 16:
        // Hexadecimal
        buffer[i++] = u'x';
        buffer[i++] = u'0';
        break;

    default:
        // Maybe invalid base, but we'll go with it (no special processing)
        break;
    }

    // NULL terminate string
    buffer[i--] = u'\0';

    // Reverse buffer before printing
    for (USize j = 0; j < i; j++, i--) {
        // Swap digits
        USize temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    globals.st->con_out->output_string(globals.st->con_out, buffer);
}

static U16 charstr[2] = {0};
void printAsci(unsigned char *string) {
    while (*string != '\0') {
        char ch = *string++;
        if (ch == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            charstr[0] = (U16)ch;
            globals.st->con_out->output_string(globals.st->con_out, charstr);
        }
    }
}

void printAsciSize(unsigned char *string, USize size) {
    for (USize i = 0; i < size; i++) {
        char ch = string[i];
        if (ch == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            charstr[0] = (U16)ch;
            globals.st->con_out->output_string(globals.st->con_out, charstr);
        }
    }
}
