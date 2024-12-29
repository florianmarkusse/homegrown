#ifndef EFI_ACPI_GUID_H
#define EFI_ACPI_GUID_H

#include "shared/types/types.h"

/**
 * Guid: Globally Unique Identifier Type
 *
 * The Guid type represents a GUID. It is always 128bit in size and
 * aligned to 64bit. Only its binary representation is guaranteed to be stable.
 * You are highly recommended to only ever access the `u8' version of it.
 *
 * The @ms1 to @ms4 fields can be used to encode Microsoft-style GUIDs, where
 * @ms1, @ms2, and @ms3 are little-endian encoded.
 */
typedef struct Guid {
    union {
        _Alignas(8) U8 u8[16];
        _Alignas(8) U16 u16[8];
        _Alignas(8) U32 u32[4];
        _Alignas(8) U64 u64[2];
        struct {
            _Alignas(8) U32 ms1;
            U16 ms2;
            U16 ms3;
            U8 ms4[8];
        };
    };
} Guid;

static constexpr auto EFI_ACPI_20_TABLE_GUID =
    (Guid){.ms1 = 0x8868e871,
           .ms2 = 0xe4f1,
           .ms3 = 0x11d3,
           .ms4 = {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
static constexpr auto ACPI_TABLE_GUID =
    (Guid){.ms1 = 0xeb9d2d30,
           .ms2 = 0x2d88,
           .ms3 = 0x11d3,
           .ms4 = {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};

#endif
