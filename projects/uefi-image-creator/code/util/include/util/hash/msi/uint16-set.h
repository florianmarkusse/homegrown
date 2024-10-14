#ifndef UTIL_HASH_MSI_UINT16_SET_H
#define UTIL_HASH_MSI_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "interoperation/types.h" // for MSI_SET
#include "util/hash/msi/common.h" // for MSI_SET
#include "util/macros.h"          // for FLO_MACRO_VAR

typedef MSI_SET(U16) msi_U16;

bool flo_msi_insertUint16(U16 value, U64 hash, msi_U16 *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool flo_msi_containsUint16(U16 value, U64 hash, msi_U16 *index);

#define FLO_FOR_EACH_MSI_UINT16(element, msiSet)                               \
    for (U64 FLO_MACRO_VAR(_index) = 0;                                        \
         FLO_MACRO_VAR(_index) < (1 << (msiSet)->exp);                         \
         ++FLO_MACRO_VAR(_index))                                              \
        if (((element) = (msiSet)->buf[FLO_MACRO_VAR(_index)]) != 0)

#ifdef __cplusplus
}
#endif

#endif
