#ifndef EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_INPUT_H
#define EFI_C_EFI_PROTOCOL_SIMPLE_TEXT_INPUT_H

#pragma once

/**
 * UEFI Protocol - Simple Text Input
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_SIMPLE_TEXT_INPUT_PROTOCOL_GUID                                  \
    C_EFI_GUID(0x387477c1, 0x69c7, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

typedef struct CEfiInputKey {
    U16 scan_code;
    U16 unicode_char;
} CEfiInputKey;

typedef struct CEfiSimpleTextInputProtocol {
    CEfiStatus(CEFICALL *reset)(CEfiSimpleTextInputProtocol *this_,
                                bool extended_verification);
    CEfiStatus(CEFICALL *read_key_stroke)(CEfiSimpleTextInputProtocol *this_,
                                          CEfiInputKey *key);
    CEfiEvent wait_for_key;
} CEfiSimpleTextInputProtocol;

#ifdef __cplusplus
}
#endif

#endif
