#ifndef EFI_FIRMWARE_DEVICE_PATH_H
#define EFI_FIRMWARE_DEVICE_PATH_H

#pragma once

/**
 * UEFI Protocol - Device Path
 *
 * XXX
 */

#include "efi/acpi/guid.h"
#include "efi/firmware/base.h"

static constexpr auto DEVICE_PATH_PROTOCOL_GUID =
    (GUID){.ms1 = 0x09576e91,
           .ms2 = 0x6d3f,
           .ms3 = 0x11d2,
           .ms4 = {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

static constexpr U8 DEVICE_PATH_TYPE_HARDWARE = 0x01;
static constexpr U8 DEVICE_PATH_TYPE_ACPI = 0x02;
static constexpr U8 DEVICE_PATH_TYPE_MESSAGE = 0x03;
static constexpr U8 DEVICE_PATH_TYPE_MEDIA = 0x04;
static constexpr U8 DEVICE_PATH_TYPE_BIOS = 0x05;
static constexpr U8 DEVICE_PATH_TYPE_END = 0x7f;

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
 * accepts nullptr. That is, the empty device-path is represented by
 * DEVICE_PATH_nullptr (which is just a TYPE_END+SUBTYPE_END_ALL). Though,
 * the UEFI Specification contradicts itself there and uses nullptr in several
 * cases. Make sure to check each of these use-cases carefully.
 */
typedef struct DevicePathProtocol {
    U8 type;
    U8 subtype;
    U8 length[2];
} DevicePathProtocol;

static constexpr U8 DEVICE_PATH_SUBTYPE_END_ALL = 0xff;
static constexpr U8 DEVICE_PATH_SUBTYPE_END_INSTANCE = 0x01;

static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_PCI = 0x01;
static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_PCCARD = 0x02;
static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_MMAP = 0x03;
static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_VENDOR = 0x04;
static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_CONTROLLER = 0x05;
static constexpr U8 DEVICE_PATH_SUBTYPE_HARDWARE_BMC = 0x06;

static constexpr auto DEVICE_PATH_nullptr = (DevicePathProtocol){
    .type = DEVICE_PATH_TYPE_END,
    .subtype = DEVICE_PATH_SUBTYPE_END_ALL,
    .length = {4, 0},
};

#endif
