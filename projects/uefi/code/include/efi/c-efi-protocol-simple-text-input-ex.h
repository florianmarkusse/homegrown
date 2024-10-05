#ifndef EFI_PROTOCOL_SIMPLE_TEXT_INPUT_EX_H
#define EFI_PROTOCOL_SIMPLE_TEXT_INPUT_EX_H

#pragma once

/**
 * UEFI Protocol - Simple Text Input Ex
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "acpi/guid.h"
#include "c-efi-base.h"
#include "efi/c-efi-protocol-simple-text-input.h"

typedef struct SimpleTextInputExProtocol SimpleTextInputExProtocol;

static constexpr auto SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID =
    (Guid){.ms1 = 0xdd9e7534,
           .ms2 = 0x7762,
           .ms3 = 0x4698,
           .ms4 = {0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, 0x25, 0xaa}};

static constexpr U8 TOGGLE_STATE_VALID = 0x80;
static constexpr U8 KEY_STATE_EXPOSED = 0x40;
static constexpr U8 SCROLL_LOCK_ACTIVE = 0x01;
static constexpr U8 NUM_LOCK_ACTIVE = 0x02;
static constexpr U8 CAPS_LOCK_ACTIVE = 0x04;

typedef U8 KeyToggleState;

static constexpr U32 SHIFT_STATE_VALID = 0x80000000;
static constexpr U32 RIGHT_SHIFT_PRESSED = 0x00000001;
static constexpr U32 LEFT_SHIFT_PRESSED = 0x00000002;
static constexpr U32 RIGHT_CONTROL_PRESSED = 0x00000004;
static constexpr U32 LEFT_CONTROL_PRESSED = 0x00000008;
static constexpr U32 RIGHT_ALT_PRESSED = 0x00000010;
static constexpr U32 LEFT_ALT_PRESSED = 0x00000020;
static constexpr U32 RIGHT_LOGO_PRESSED = 0x00000040;
static constexpr U32 LEFT_LOGO_PRESSED = 0x00000080;
static constexpr U32 MENU_KEY_PRESSED = 0x00000100;
static constexpr U32 SYS_REQ_PRESSED = 0x00000200;

typedef struct KeyState {
    U32 key_shift_state;
    KeyToggleState key_toggle_state;
} KeyState;

typedef struct KeyData {
    InputKey key;
    KeyState key_state;
} KeyData;

typedef Status(EFICALL *KeyNotifyFunction)(KeyData *key_data);

typedef struct SimpleTextInputExProtocol {
    Status(EFICALL *reset)(SimpleTextInputExProtocol *this_,
                           bool extended_verification);
    Status(EFICALL *read_key_stroke_ex)(SimpleTextInputExProtocol *this_,
                                        KeyData *key_data);
    Event wait_for_key_ex;
    Status(EFICALL *set_state)(SimpleTextInputExProtocol *this_,
                               KeyToggleState *key_toggle_state);
    Status(EFICALL *register_key_notify)(
        SimpleTextInputExProtocol *this_, KeyData *key_data,
        KeyNotifyFunction key_notification_function, void **notify_handle);
    Status(EFICALL *unregister_key_notify)(SimpleTextInputExProtocol *this_,
                                           void *notification_handle);
} SimpleTextInputExProtocol;

#ifdef __cplusplus
}
#endif

#endif
