#ifndef EFI_PROTOCOL_SIMPLE_TEXT_OUTPUT_H
#define EFI_PROTOCOL_SIMPLE_TEXT_OUTPUT_H

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

#define SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID                                 \
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

#define BLACK U8_C(0x00)
#define BLUE U8_C(0x01)
#define GREEN U8_C(0x02)
#define CYAN U8_C(0x03)
#define RED U8_C(0x04)
#define MAGENTA U8_C(0x05)
#define BROWN U8_C(0x06)
#define LIGHTGRAY U8_C(0x07)
#define BRIGHT U8_C(0x08)
#define DARKGRAY (BLACK | BRIGHT)
#define LIGHTBLUE (BLUE | BRIGHT)
#define LIGHTGREEN (GREEN | BRIGHT)
#define LIGHTCYAN (CYAN | BRIGHT)
#define LIGHTRED (RED | BRIGHT)
#define LIGHTMAGENTA (MAGENTA | BRIGHT)
#define YELLOW (BROWN | BRIGHT)
#define WHITE U8_C(0x0f)

#define BACKGROUND_BLACK U8_C(0x00)
#define BACKGROUND_BLUE U8_C(0x10)
#define BACKGROUND_GREEN U8_C(0x20)
#define BACKGROUND_CYAN U8_C(0x30)
#define BACKGROUND_RED U8_C(0x40)
#define BACKGROUND_MAGENTA U8_C(0x50)
#define BACKGROUND_BROWN U8_C(0x60)
#define BACKGROUND_LIGHTGRAY U8_C(0x70)

typedef struct SimpleTextOutputProtocol {
    Status(EFICALL *reset)(SimpleTextOutputProtocol *this_,
                                bool extended_verification);
    Status(EFICALL *output_string)(SimpleTextOutputProtocol *this_,
                                        U16 *string);
    Status(EFICALL *test_string)(SimpleTextOutputProtocol *this_,
                                      U16 *string);
    Status(EFICALL *query_mode)(SimpleTextOutputProtocol *this_,
                                     USize mode_number, USize *columns,
                                     USize *rows);
    Status(EFICALL *set_mode)(SimpleTextOutputProtocol *this_,
                                   USize mode_number);
    Status(EFICALL *set_attribute)(SimpleTextOutputProtocol *this_,
                                        USize attribute);
    Status(EFICALL *clear_screen)(SimpleTextOutputProtocol *this_);
    Status(EFICALL *set_cursor_position)(
        SimpleTextOutputProtocol *this_, USize column, USize row);
    Status(EFICALL *enable_cursor)(SimpleTextOutputProtocol *this_,
                                        bool visible);
    SimpleTextOutputMode *mode;
} SimpleTextOutpuProtocol;

#ifdef __cplusplus
}
#endif

#endif
