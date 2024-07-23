#ifndef EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_INPUT_EX_H
#define EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_INPUT_EX_H

#pragma once

/**
 * UEFI Protocol - Simple Text Input Ex
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

typedef struct SimpleTextInputExProtocol SimpleTextInputExProtocol;

#define C_EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID                               \
    EFI_GUID(0xdd9e7534, 0x7762, 0x4698, 0x8c, 0x14, 0xf5, 0x85, 0x17, 0xa6, \
               0x25, 0xaa)

#define C_EFI_TOGGLE_STATE_VALID U8_C(0x80)
#define C_EFI_KEY_STATE_EXPOSED U8_C(0x40)
#define C_EFI_SCROLL_LOCK_ACTIVE U8_C(0x01)
#define C_EFI_NUM_LOCK_ACTIVE U8_C(0x02)
#define C_EFI_CAPS_LOCK_ACTIVE U8_C(0x04)

typedef U8 KeyToggleState;

#define C_EFI_SHIFT_STATE_VALID U32_C(0x80000000)
#define C_EFI_RIGHT_SHIFT_PRESSED U32_C(0x00000001)
#define C_EFI_LEFT_SHIFT_PRESSED U32_C(0x00000002)
#define C_EFI_RIGHT_CONTROL_PRESSED U32_C(0x00000004)
#define C_EFI_LEFT_CONTROL_PRESSED U32_C(0x00000008)
#define C_EFI_RIGHT_ALT_PRESSED U32_C(0x00000010)
#define C_EFI_LEFT_ALT_PRESSED U32_C(0x00000020)
#define C_EFI_RIGHT_LOGO_PRESSED U32_C(0x00000040)
#define C_EFI_LEFT_LOGO_PRESSED U32_C(0x00000080)
#define C_EFI_MENU_KEY_PRESSED U32_C(0x00000100)
#define C_EFI_SYS_REQ_PRESSED U32_C(0x00000200)

typedef struct KeyState {
    U32 key_shift_state;
    KeyToggleState key_toggle_state;
} KeyState;

typedef struct KeyData {
    InputKey key;
    KeyState key_state;
} KeyData;

typedef Status(CEFICALL *KeyNotifyFunction)(KeyData *key_data);

typedef struct SimpleTextInputExProtocol {
    Status(CEFICALL *reset)(SimpleTextInputExProtocol *this_,
                                bool extended_verification);
    Status(CEFICALL *read_key_stroke_ex)(
        SimpleTextInputExProtocol *this_, KeyData *key_data);
    Event wait_for_key_ex;
    Status(CEFICALL *set_state)(SimpleTextInputExProtocol *this_,
                                    KeyToggleState *key_toggle_state);
    Status(CEFICALL *register_key_notify)(
        SimpleTextInputExProtocol *this_, KeyData *key_data,
        KeyNotifyFunction key_notification_function, void **notify_handle);
    Status(CEFICALL *unregister_key_notify)(
        SimpleTextInputExProtocol *this_, void *notification_handle);
} SimpleTextInputExProtocol;

#ifdef __cplusplus
}
#endif

#endif
