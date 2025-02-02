#ifndef EFI_FIRMWARE_DEVICE_PATH_UTILITY_H
#define EFI_FIRMWARE_DEVICE_PATH_UTILITY_H

#pragma once

/**
 * UEFI Protocol - Device Path Utility
 *
 * XXX
 */

#include "efi/firmware/base.h"
#include "efi/firmware/device-path.h"

static constexpr auto DEVICE_PATH_UTILITIES_PROTOCOL_GUID =
    (GUID){.ms1 = 0x379be4e,
           .ms2 = 0xd706,
           .ms3 = 0x437d,
           .ms4 = {0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7, 0x72, 0xa4}};

typedef struct DevicePathUtilitiesProtocol {
    USize(EFICALL *get_device_path_size)(DevicePathProtocol *device_path);
    DevicePathProtocol *(EFICALL *duplicate_device_path)(
        DevicePathProtocol *device_path);
    DevicePathProtocol *(EFICALL *append_device_path)(DevicePathProtocol *src1,
                                                      DevicePathProtocol *src2);
    DevicePathProtocol *(EFICALL *append_device_node)(
        DevicePathProtocol *device_path, DevicePathProtocol *device_node);
    DevicePathProtocol *(EFICALL *append_device_path_instance)(
        DevicePathProtocol *device_path,
        DevicePathProtocol *device_path_instance);
    DevicePathProtocol *(EFICALL *get_next_device_path_instance)(
        DevicePathProtocol **device_path_instance,
        USize *device_path_instance_size);
    bool(EFICALL *is_device_path_multi_instance)(
        DevicePathProtocol *device_path);
    DevicePathProtocol *(EFICALL *create_device_node)(U8 node_type,
                                                      U8 node_subtype,
                                                      U16 node_length);
} DevicePathUtilitiesProtocol;

#endif
