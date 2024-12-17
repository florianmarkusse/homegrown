#include "printing.h"
#include "efi/c-efi-base.h"                        // for SUCCESS
#include "efi/c-efi-protocol-simple-text-input.h"  // for InputKey
#include "efi/c-efi-protocol-simple-text-output.h" // for SimpleTextOutputP...
#include "efi/c-efi-system.h"                      // for ResetType
#include "globals.h"                               // for globals

void error(U16 *string) {
    globals.st->con_out->output_string(globals.st->con_out, string);
    InputKey key;
    while (globals.st->con_in->read_key_stroke(globals.st->con_in, &key) !=
           SUCCESS) {
        ;
    }
    globals.st->runtime_services->reset_system(RESET_SHUTDOWN, SUCCESS, 0,
                                               nullptr);
}

static const U16 *digits = u"0123456789ABCDEF";
void printNumber(USize number, U8 base) {
    U16 buffer[24]; // Hopefully enough for UINTN_MAX (UINT64_MAX) + sign
                    // I8acter
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

    // nullptr terminate string
    buffer[i--] = u'\0';

    // Reverse buffer before printing
    for (U16 j = 0; j < i; j++, i--) {
        // Swap digits
        U16 temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
    }

    globals.st->con_out->output_string(globals.st->con_out, buffer);
}

static U16 I8str[2] = {0};
void printAsci(U8 *string) {
    while (*string != '\0') {
        I8 ch = *string++;
        if (ch == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            I8str[0] = (U16)ch;
            globals.st->con_out->output_string(globals.st->con_out, I8str);
        }
    }
}

void printAsciSize(U8 *string, USize size) {
    for (USize i = 0; i < size; i++) {
        I8 ch = string[i];
        if (ch == '\n') {
            globals.st->con_out->output_string(globals.st->con_out, u"\r\n");
        } else {
            I8str[0] = (U16)ch;
            globals.st->con_out->output_string(globals.st->con_out, I8str);
        }
    }
}
