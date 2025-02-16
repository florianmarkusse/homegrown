#ifndef EFI_FIRMWARE_DEVICE_PATH_TO_TEXT_H
#define EFI_FIRMWARE_DEVICE_PATH_TO_TEXT_H

/**
 * UEFI Protocol - Device Path To Text
 *
 * XXX
 */

#include "efi/acpi/guid.h"
#include "efi/firmware/base.h"
#include "efi/firmware/device-path.h"

static constexpr auto DEVICE_PATH_TO_TEXT_PROTOCOL_GUID =
    (GUID){.ms1 = 0x8b843e20,
           .ms2 = 0x8132,
           .ms3 = 0x4852,
           .ms4 = {0x90, 0xcc, 0x55, 0x1a, 0x4e, 0x4a, 0x7f, 0x1c}};

typedef struct DevicePathToTextProtocol {
    U16 *(*convert_device_node_to_text)(DevicePathProtocol *device_node,
                                                bool display_only,
                                                bool allow_shortcuts);
    U16 *(*convert_device_path_to_text)(DevicePathProtocol *device_path,
                                                bool display_only,
                                                bool allow_shortcuts);
} DevicePathToTextProtocol;

#endif
