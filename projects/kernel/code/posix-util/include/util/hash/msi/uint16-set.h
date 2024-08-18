#ifndef UTIL_HASH_MSI_UINT16_SET_H
#define UTIL_HASH_MSI_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"      // for MSI_SET
#include "util/macros.h" // for MACRO_VAR
#include <stddef.h>      // for size_t
#include <stdint.h>      // for uint16_t

typedef MSI_SET(uint16_t) msi_Uint16;

bool msi_insertUint16(uint16_t value, size_t hash, msi_Uint16 *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool msi_containsUint16(uint16_t value, size_t hash, msi_Uint16 *index);

#define FOR_EACH_MSI_UINT16(element, msiSet)                               \
    for (ptrdiff_t MACRO_VAR(_index) = 0;                                  \
         MACRO_VAR(_index) < (1 << (msiSet)->exp);                         \
         ++MACRO_VAR(_index))                                              \
        if (((element) = (msiSet)->buf[MACRO_VAR(_index)]) != 0)

#ifdef __cplusplus
}
#endif

#endif
