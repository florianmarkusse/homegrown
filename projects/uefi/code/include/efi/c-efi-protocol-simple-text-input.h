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

#include "acpi/guid.h"
#include "c-efi-base.h"

static constexpr auto SIMPLE_TEXT_INPUT_PROTOCOL_GUID =
    (Guid){.ms1 = 0x387477c1,
           .ms2 = 0x69c7,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

typedef struct InputKey {
    U16 scan_code;
    U16 unicode_I8;
} InputKey;

typedef struct SimpleTextInputProtocol {
    Status(EFICALL *reset)(SimpleTextInputProtocol *this_,
                           bool extended_verification);
    Status(EFICALL *read_key_stroke)(SimpleTextInputProtocol *this_,
                                     InputKey *key);
    Event wait_for_key;
} SimpleTextInputProtocol;

#ifdef __cplusplus
}
#endif

#endif
