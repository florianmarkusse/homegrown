#ifndef EFI_FIRMWARE_LOADED_IMAGE_DEVICE_PATH_H
#define EFI_FIRMWARE_LOADED_IMAGE_DEVICE_PATH_H

#pragma once

/**
 * UEFI Protocol - Loaded Image Device Path
 *
 * XXX
 */

#include "efi/acpi/guid.h"

static constexpr auto LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID =
    (GUID){.ms1 = 0xbc62157e,
           .ms2 = 0x3e33,
           .ms3 = 0x4fec,
           .ms4 = {0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf}};

#endif
