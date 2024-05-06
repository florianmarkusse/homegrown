#ifndef ACPI_GUID_H
#define ACPI_GUID_H

#include "types.h"

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

#define EFI_GUID(_ms1, _ms2, _ms3, _ms4, _ms5, _ms6, _ms7, _ms8, _ms9, _ms10,  \
                 _ms11)                                                        \
    ((struct Guid){                                                            \
        .ms1 = (_ms1),                                                         \
        .ms2 = (_ms2),                                                         \
        .ms3 = (_ms3),                                                         \
        .ms4 =                                                                 \
            {                                                                  \
                (_ms4),                                                        \
                (_ms5),                                                        \
                (_ms6),                                                        \
                (_ms7),                                                        \
                (_ms8),                                                        \
                (_ms9),                                                        \
                (_ms10),                                                       \
                (_ms11),                                                       \
            },                                                                 \
    })

#define C_EFI_EFI_ACPI_20_TABLE_GUID                                           \
    EFI_GUID(0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c,   \
             0x88, 0x81)
#define C_EFI_ACPI_TABLE_GUID                                                  \
    EFI_GUID(0xeb9d2d30, 0x2d88, 0x11d3, 0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f,   \
             0xc1, 0x4d)

#endif
