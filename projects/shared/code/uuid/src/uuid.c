#include "shared/uuid.h"
#include "shared/types/types.h"

void setUUIDType(UUID *uuid, U8 version, UUIDVariant variant) {
    // The first 4 bits indicate the version.
    // Move the version in while masking the bottom 12 bits
    uuid->timeHiAndVer = ((U8)(version << 12)) | (uuid->timeHiAndVer & 0x0FFF);

    switch (variant) {
    case UUID_VARIANT_0: {
        // 0 x x x x x x x
        uuid->clockSeqHiAndRes =
            ((U8)(0b0 << 7)) | (uuid->clockSeqHiAndRes & 0b01111111);
        break;
    }
    case UUID_VARIANT_1: {
        // 1 0 x x x x x x
        uuid->clockSeqHiAndRes =
            ((U8)(0b10 << 6)) | (uuid->clockSeqHiAndRes & 0b00111111);
        break;
    }
    case UUID_VARIANT_2: {
        // 1 1 0 x x x x x
        uuid->clockSeqHiAndRes =
            ((U8)(0b110 << 5)) | (uuid->clockSeqHiAndRes & 0b00011111);
        break;
    }
    case UUID_VARIANT_3: {
        // 1 1 1 x x x x x
        uuid->clockSeqHiAndRes =
            ((U8)(0b111 << 5)) | (uuid->clockSeqHiAndRes & 0b00011111);
        break;
    }
    }
}
