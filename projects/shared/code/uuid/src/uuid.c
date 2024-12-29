#include "shared/uuid.h"
#include "shared/types/types.h"

void setUUIDType(UUID *uuid, U8 version, UUIDVariant variant) {
    // The first 4 bits indicate the version.
    // Move the version in while masking the bottom 4 bits
    uuid->versionAnd52To55 =
        ((U8)(version << 4)) | (uuid->versionAnd52To55 & 0b00001111);
    // The first 3 bits or fewer indicate the version.
    // Move the version in while masking the bottom 5 bits
    uuid->variantAnd67To71 =
        ((U8)(variant << 3)) | (uuid->variantAnd67To71 & 0b00011111);
}
