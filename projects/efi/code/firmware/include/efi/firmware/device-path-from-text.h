#ifndef EFI_C_EFI_PROTOCOL_DEVICE_PATH_FROM_TEXT_H
#define EFI_C_EFI_PROTOCOL_DEVICE_PATH_FROM_TEXT_H

#pragma once

/**
 * UEFI Protocol - Device Path From Text
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "efi/acpi/guid.h"
#include "efi/efi/c-efi-base.h"
#include "efi/efi/c-efi-protocol-device-path.h"

static constexpr auto DEVICE_PATH_FROM_TEXT_PROTOCOL_GUID =
    (GUID){.ms1 = 0x5c99a21,
           .ms2 = 0xc70f,
           .ms3 = 0x4ad2,
           .ms4 = {0x8a, 0x5f, 0x35, 0xdf, 0x33, 0x43, 0xf5, 0x1e}};

typedef struct DevicePathFromTextProtocol {
    DevicePathProtocol *(EFICALL *convert_text_to_device_node)(
        U16 *text_device_node);
    DevicePathProtocol *(EFICALL *convert_text_to_device_path)(
        U16 *text_device_path);
} DevicePathFromTextProtocol;

#ifdef __cplusplus
}
#endif

#endif
