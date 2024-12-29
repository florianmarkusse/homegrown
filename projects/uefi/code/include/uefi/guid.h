#ifndef UEFI_GUID_H
#define UEFI_GUID_H

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

#endif
