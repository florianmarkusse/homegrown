#ifndef EFI_PROTOCOL_DEVICE_PATH_H
#define EFI_PROTOCOL_DEVICE_PATH_H

#pragma once

/**
 * UEFI Protocol - Device Path
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define DEVICE_PATH_PROTOCOL_GUID                                        \
    EFI_GUID(0x09576e91, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, \
               0x72, 0x3b)

#define DEVICE_PATH_TYPE_HARDWARE U8_C(0x01)
#define DEVICE_PATH_TYPE_ACPI U8_C(0x02)
#define DEVICE_PATH_TYPE_MESSAGE U8_C(0x03)
#define DEVICE_PATH_TYPE_MEDIA U8_C(0x04)
#define DEVICE_PATH_TYPE_BIOS U8_C(0x05)
#define DEVICE_PATH_TYPE_END U8_C(0x7f)

/**
 * DevicePathProtocol: Device Paths
 * @type:               type of this device node
 * @subtype:            subtype of this device node
 * @length:             length of this device node (including this header)
 *
 * This structure is used to represent paths to all kinds of devices. A device
 * path is a concatenation of structures of this type. The end is marked with a
 * type/subtype combination of TYPE_END and SUBTYPE_END_ALL.
 *
 * Note that thus structure is unaligned! That is, its alignment is 1-byte and
 * thus must be accessed with unaligned helpers, or in individual pieces.
 *
 * Also note that any function taking an object of this type usually never
 * accepts NULL. That is, the empty device-path is represented by
 * DEVICE_PATH_NULL (which is just a TYPE_END+SUBTYPE_END_ALL). Though,
 * the UEFI Specification contradicts itself there and uses NULL in several
 * cases. Make sure to check each of these use-cases carefully.
 */
typedef struct DevicePathProtocol {
    U8 type;
    U8 subtype;
    U8 length[2];
} DevicePathProtocol;

#define DEVICE_PATH_SUBTYPE_END_ALL U8_C(0xff)
#define DEVICE_PATH_SUBTYPE_END_INSTANCE U8_C(0x01)

#define DEVICE_PATH_SUBTYPE_HARDWARE_PCI U8_C(0x01)
#define DEVICE_PATH_SUBTYPE_HARDWARE_PCCARD U8_C(0x02)
#define DEVICE_PATH_SUBTYPE_HARDWARE_MMAP U8_C(0x03)
#define DEVICE_PATH_SUBTYPE_HARDWARE_VENDOR U8_C(0x04)
#define DEVICE_PATH_SUBTYPE_HARDWARE_CONTROLLER U8_C(0x05)
#define DEVICE_PATH_SUBTYPE_HARDWARE_BMC U8_C(0x06)

#define DEVICE_PATH_NULL                                                 \
    {                                                                          \
        .type = DEVICE_PATH_TYPE_END,                                    \
        .subtype = DEVICE_PATH_SUBTYPE_END_ALL, .length = {4, 0},        \
    }

#ifdef __cplusplus
}
#endif

#endif
