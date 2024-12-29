#ifndef EFI_EFI_C_EFI_PROTOCOL_LOADED_IMAGE_DEVICE_PATH_H
#define EFI_EFI_C_EFI_PROTOCOL_LOADED_IMAGE_DEVICE_PATH_H

#pragma once

/**
 * UEFI Protocol - Loaded Image Device Path
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "efi/acpi/guid.h"

static constexpr auto LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID =
    (Guid){.ms1 = 0xbc62157e,
           .ms2 = 0x3e33,
           .ms3 = 0x4fec,
           .ms4 = {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf}};

#ifdef __cplusplus
}
#endif

#endif
