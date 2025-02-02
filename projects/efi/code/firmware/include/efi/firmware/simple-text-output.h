#ifndef EFI_FIRMWARE_SIMPLE_TEXT_OUTPUT_H
#define EFI_FIRMWARE_SIMPLE_TEXT_OUTPUT_H

/**
 * UEFI Protocol - Simple Text Output
 *
 * XXX
 */

#include "efi/firmware/base.h"
#include "shared/uuid.h"

static constexpr auto SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID =
    (UUID){.ms1 = 0x387477c2,
           .ms2 = 0x69c7,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

typedef struct SimpleTextOutputMode {
    I32 max_mode;
    I32 mode;
    I32 attribute;
    I32 cursor_column;
    I32 cursor_row;
    bool cursor_visible;
} SimpleTextOutputMode;

static constexpr U8 BLACK = 0x00;
static constexpr U8 BLUE = 0x01;
static constexpr U8 GREEN = 0x02;
static constexpr U8 CYAN = 0x03;
static constexpr U8 RED = 0x04;
static constexpr U8 MAGENTA = 0x05;
static constexpr U8 BROWN = 0x06;
static constexpr U8 LIGHTGRAY = 0x07;
static constexpr U8 BRIGHT = 0x08;
static constexpr U8 DARKGRAY = (BLACK | BRIGHT);
static constexpr U8 LIGHTBLUE = (BLUE | BRIGHT);
static constexpr U8 LIGHTGREEN = (GREEN | BRIGHT);
static constexpr U8 LIGHTCYAN = (CYAN | BRIGHT);
static constexpr U8 LIGHTRED = (RED | BRIGHT);
static constexpr U8 LIGHTMAGENTA = (MAGENTA | BRIGHT);
static constexpr U8 YELLOW = (BROWN | BRIGHT);
static constexpr U8 WHITE = 0x0f;

static constexpr U8 BACKGROUND_BLACK = 0x00;
static constexpr U8 BACKGROUND_BLUE = 0x10;
static constexpr U8 BACKGROUND_GREEN = 0x20;
static constexpr U8 BACKGROUND_CYAN = 0x30;
static constexpr U8 BACKGROUND_RED = 0x40;
static constexpr U8 BACKGROUND_MAGENTA = 0x50;
static constexpr U8 BACKGROUND_BROWN = 0x60;
static constexpr U8 BACKGROUND_LIGHTGRAY = 0x70;

typedef struct SimpleTextOutputProtocol {
    Status(EFICALL *reset)(SimpleTextOutputProtocol *this_,
                           bool extended_verification);
    Status(EFICALL *output_string)(SimpleTextOutputProtocol *this_,
                                   U16 *string);
    Status(EFICALL *test_string)(SimpleTextOutputProtocol *this_, U16 *string);
    Status(EFICALL *query_mode)(SimpleTextOutputProtocol *this_,
                                USize mode_number, USize *columns, USize *rows);
    Status(EFICALL *set_mode)(SimpleTextOutputProtocol *this_,
                              USize mode_number);
    Status(EFICALL *set_attribute)(SimpleTextOutputProtocol *this_,
                                   USize attribute);
    Status(EFICALL *clear_screen)(SimpleTextOutputProtocol *this_);
    Status(EFICALL *set_cursor_position)(SimpleTextOutputProtocol *this_,
                                         USize column, USize row);
    Status(EFICALL *enable_cursor)(SimpleTextOutputProtocol *this_,
                                   bool visible);
    SimpleTextOutputMode *mode;
} SimpleTextOutpuProtocol;

#endif
