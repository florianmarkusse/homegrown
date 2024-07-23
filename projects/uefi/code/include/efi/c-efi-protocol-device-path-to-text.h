#ifndef EFI_C_EFI_PROTOCOL_DEVICE_PATH_TO_TEXT_H
#define EFI_C_EFI_PROTOCOL_DEVICE_PATH_TO_TEXT_H

#pragma once

/**
 * UEFI Protocol - Device Path To Text
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-protocol-device-path.h"

#define C_EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID                                \
    C_EFI_GUID(0x8b843e20, 0x8132, 0x4852, 0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, \
               0x7f, 0x1c)

typedef struct CEfiDevicePathToTextProtocol {
    U16 *(CEFICALL *convert_device_node_to_text)(
        CEfiDevicePathProtocol *device_node, bool display_only,
        bool allow_shortcuts);
    U16 *(CEFICALL *convert_device_path_to_text)(
        CEfiDevicePathProtocol *device_path, bool display_only,
        bool allow_shortcuts);
} CEfiDevicePathToTextProtocol;

#ifdef __cplusplus
}
#endif

#endif
