#ifndef SHARED_UUID_H
#define SHARED_UUID_H

#include "shared/types/types.h"

// NOTE: Ready for code generation
// To create a UUID based on the string representation instead
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
            U32 timeLo;
            U16 timeMid;
            U16 timeHiAndVer;    // Highest 4 bits are version #
            U8 clockSeqHiAndRes; // Highest bits are variant #
            U8 clockSeqLo;
            U8 node[6];
        };
    };
} UUID;

typedef enum {
    UUID_VARIANT_0,
    UUID_VARIANT_1,
    UUID_VARIANT_2,
    UUID_VARIANT_3,
} UUIDVariant;

static constexpr UUID NIL_UUID = {0};
static constexpr UUID MAX_UUID = {
    .u64 = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF}};

void setUUIDType(UUID *uuid, U8 version, UUIDVariant variant);

#endif
