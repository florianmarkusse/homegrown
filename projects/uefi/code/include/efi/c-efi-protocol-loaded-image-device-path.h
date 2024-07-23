#ifndef EFI_PROTOCOL_LOADED_IMAGE_DEVICE_PATH_H
#define EFI_PROTOCOL_LOADED_IMAGE_DEVICE_PATH_H

#pragma once

/**
 * UEFI Protocol - Loaded Image Device Path
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID                           \
    EFI_GUID(0xbc62157e, 0x3e33, 0x4fec, 0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, \
               0x50, 0xdf)

#ifdef __cplusplus
}
#endif

#endif
