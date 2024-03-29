#ifndef UTIL_HASH_MSI_UINT16_SET_H
#define UTIL_HASH_MSI_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"      // for FLO_MSI_SET
#include "util/macros.h" // for FLO_MACRO_VAR
#include <stddef.h>      // for size_t
#include <stdint.h>      // for uint16_t

typedef FLO_MSI_SET(uint16_t) flo_msi_Uint16;

bool flo_msi_insertUint16(uint16_t value, size_t hash, flo_msi_Uint16 *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool flo_msi_containsUint16(uint16_t value, size_t hash, flo_msi_Uint16 *index);

#define FLO_FOR_EACH_MSI_UINT16(element, msiSet)                               \
    for (ptrdiff_t FLO_MACRO_VAR(_index) = 0;                                  \
         FLO_MACRO_VAR(_index) < (1 << (msiSet)->exp);                         \
         ++FLO_MACRO_VAR(_index))                                              \
        if (((element) = (msiSet)->buf[FLO_MACRO_VAR(_index)]) != 0)

#ifdef __cplusplus
}
#endif

#endif
