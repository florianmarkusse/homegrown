#ifndef EFI_PROTOCOL_LOADED_IMAGE_H
#define EFI_PROTOCOL_LOADED_IMAGE_H

#pragma once

/**
 * UEFI Protocol - Loaded Image
 *
 * XXX
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"
#include "c-efi-system.h"

static constexpr auto LOADED_IMAGE_PROTOCOL_GUID =
    (Guid){.ms1 = 0x5B1B31A1,
           .ms2 = 0x9562,
           .ms3 = 0x11d2,
           .ms4 = {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};

static constexpr U32 LOADED_IMAGE_PROTOCOL_REVISION = 0x1000;

typedef struct LoadedImageProtocol {
    U32 revision;
    Handle parent_handle;
    SystemTable *system_table;

    Handle device_handle;
    DevicePathProtocol *file_path;
    void *reserved;

    U32 load_options_size;
    void *load_options;

    void *image_base;
    U64 image_size;
    MemoryType image_code_type;
    MemoryType image_data_type;

    Status(EFICALL *unload)(Handle image_handle);
} LoadedImageProtocol;

#ifdef __cplusplus
}
#endif

#endif
