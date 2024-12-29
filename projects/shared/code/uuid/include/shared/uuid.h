#ifndef SHARED_UUID_H
#define SHARED_UUID_H

#include "shared/types/types.h"

/**
 * UUID: Universally Unique Identifier Type
 *
 * The UUID type represents a UUID. It is always 128bit in size and
 * aligned to 64bit. Only its binary representation is guaranteed to be stable.
 *
 * The @ms1 to @ms4 fields can be used to encode Microsoft-style UUIDs, where
 * @ms1, @ms2, and @ms3 are little-endian encoded.
 */
typedef struct UUID {
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
        struct {
            U32 first32;
            U16 from32To47;
            union {
                struct {
                    U8 versionAnd52To55; // Highest 4 bits are version #
                    U8 from56To63;
                };
                U16 from48To63;
            };
            union {
                struct {
                    U8 variantAnd67To71; // Highest 1, 2, or 3 bits are variant
                                         // #
                    U8 from72To79;
                };
                U16 from64To79;
            };
            U8 from80to127[6];
        };
    };
} UUID;

typedef enum {
    UUID_VARIANT_0 =
        0b011, // 0b0 is the only bit that matters but we are &ding it
    UUID_VARIANT_1 = 0b101, // 0b10
    UUID_VARIANT_2 = 0b110, // 0b110
    UUID_VARIANT_3 = 0b111  // 0b111
} UUIDVariant;

static constexpr UUID NIL_UUID = {0};
static constexpr UUID MAX_UUID = {
    .u64 = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}};

void setUUIDType(UUID *uuid, U8 version, UUIDVariant variant);

#endif
