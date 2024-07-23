#ifndef EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_OUTPUT_H
#define EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_OUTPUT_H

#pragma once

/**
 * UEFI Protocol - Simple Text Output
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID                                 \
    EFI_GUID(0x387477c2, 0x69c7, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct SimpleTextOutputMode {
    I32 max_mode;
    I32 mode;
    I32 attribute;
    I32 cursor_column;
    I32 cursor_row;
    bool cursor_visible;
} SimpleTextOutputMode;

#define C_EFI_BLACK U8_C(0x00)
#define C_EFI_BLUE U8_C(0x01)
#define C_EFI_GREEN U8_C(0x02)
#define C_EFI_CYAN U8_C(0x03)
#define C_EFI_RED U8_C(0x04)
#define C_EFI_MAGENTA U8_C(0x05)
#define C_EFI_BROWN U8_C(0x06)
#define C_EFI_LIGHTGRAY U8_C(0x07)
#define C_EFI_BRIGHT U8_C(0x08)
#define C_EFI_DARKGRAY (C_EFI_BLACK | C_EFI_BRIGHT)
#define C_EFI_LIGHTBLUE (C_EFI_BLUE | C_EFI_BRIGHT)
#define C_EFI_LIGHTGREEN (C_EFI_GREEN | C_EFI_BRIGHT)
#define C_EFI_LIGHTCYAN (C_EFI_CYAN | C_EFI_BRIGHT)
#define C_EFI_LIGHTRED (C_EFI_RED | C_EFI_BRIGHT)
#define C_EFI_LIGHTMAGENTA (C_EFI_MAGENTA | C_EFI_BRIGHT)
#define C_EFI_YELLOW (C_EFI_BROWN | C_EFI_BRIGHT)
#define C_EFI_WHITE U8_C(0x0f)

#define C_EFI_BACKGROUND_BLACK U8_C(0x00)
#define C_EFI_BACKGROUND_BLUE U8_C(0x10)
#define C_EFI_BACKGROUND_GREEN U8_C(0x20)
#define C_EFI_BACKGROUND_CYAN U8_C(0x30)
#define C_EFI_BACKGROUND_RED U8_C(0x40)
#define C_EFI_BACKGROUND_MAGENTA U8_C(0x50)
#define C_EFI_BACKGROUND_BROWN U8_C(0x60)
#define C_EFI_BACKGROUND_LIGHTGRAY U8_C(0x70)

typedef struct SimpleTextOutputProtocol {
    Status(CEFICALL *reset)(SimpleTextOutputProtocol *this_,
                                bool extended_verification);
    Status(CEFICALL *output_string)(SimpleTextOutputProtocol *this_,
                                        U16 *string);
    Status(CEFICALL *test_string)(SimpleTextOutputProtocol *this_,
                                      U16 *string);
    Status(CEFICALL *query_mode)(SimpleTextOutputProtocol *this_,
                                     USize mode_number, USize *columns,
                                     USize *rows);
    Status(CEFICALL *set_mode)(SimpleTextOutputProtocol *this_,
                                   USize mode_number);
    Status(CEFICALL *set_attribute)(SimpleTextOutputProtocol *this_,
                                        USize attribute);
    Status(CEFICALL *clear_screen)(SimpleTextOutputProtocol *this_);
    Status(CEFICALL *set_cursor_position)(
        SimpleTextOutputProtocol *this_, USize column, USize row);
    Status(CEFICALL *enable_cursor)(SimpleTextOutputProtocol *this_,
                                        bool visible);
    SimpleTextOutputMode *mode;
} SimpleTextOutpuProtocol;

#ifdef __cplusplus
}
#endif

#endif
