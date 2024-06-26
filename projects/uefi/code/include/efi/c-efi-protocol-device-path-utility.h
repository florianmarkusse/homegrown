#ifndef EFI_C_EFI_PROTOCOL_DEVICE_PATH_UTILITY_H
#define EFI_C_EFI_PROTOCOL_DEVICE_PATH_UTILITY_H

#pragma once

/**
 * UEFI Protocol - Device Path Utility
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-protocol-device-path.h"

#define C_EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID                              \
    C_EFI_GUID(0x379be4e, 0xd706, 0x437d, 0xb0, 0x37, 0xed, 0xb8, 0x2f, 0xb7,  \
               0x72, 0xa4)

typedef struct CEfiDevicePathUtilitiesProtocol {
    CEfiUSize(CEFICALL *get_device_path_size)(
        CEfiDevicePathProtocol *device_path);
    CEfiDevicePathProtocol *(CEFICALL *duplicate_device_path)(
        CEfiDevicePathProtocol *device_path);
    CEfiDevicePathProtocol *(CEFICALL *append_device_path)(
        CEfiDevicePathProtocol *src1, CEfiDevicePathProtocol *src2);
    CEfiDevicePathProtocol *(CEFICALL *append_device_node)(
        CEfiDevicePathProtocol *device_path,
        CEfiDevicePathProtocol *device_node);
    CEfiDevicePathProtocol *(CEFICALL *append_device_path_instance)(
        CEfiDevicePathProtocol *device_path,
        CEfiDevicePathProtocol *device_path_instance);
    CEfiDevicePathProtocol *(CEFICALL *get_next_device_path_instance)(
        CEfiDevicePathProtocol **device_path_instance,
        CEfiUSize *device_path_instance_size);
    CEfiBool(CEFICALL *is_device_path_multi_instance)(
        CEfiDevicePathProtocol *device_path);
    CEfiDevicePathProtocol *(CEFICALL *create_device_node)(CEfiU8 node_type,
                                                           CEfiU8 node_subtype,
                                                           CEfiU16 node_length);
} CEfiDevicePathUtilitiesProtocol;

#ifdef __cplusplus
}
#endif

#endif
