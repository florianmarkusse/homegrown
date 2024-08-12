#ifndef UTIL_HASH_MSI_STRING_SET_H
#define UTIL_HASH_MSI_STRING_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"                           // for MSI_SET
#include "util/hash/hash-comparison-status.h" // for HashComparisonStatus
#include "util/macros.h"                      // for MACRO_VAR
#include "util/text/string.h"                 // for string
#include <stdbool.h>                          // for true
#include <stddef.h>                           // for ptrdiff_t, size_t

typedef MSI_SET(string) msi_String;

bool msi_insertString(string string, size_t hash,
                          msi_String *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool msi_containsString(string string, size_t hash,
                            msi_String *index);

/**
 * Uses hashStringDjb2 to compare so if any of the sets used a customs
 * hashing function, do not use!!!
 */
HashComparisonStatus msi_equalsStringSet(msi_String *set1,
                                                 msi_String *set2);

#define FOR_EACH_MSI_STRING(element, msiSet)                               \
    for (ptrdiff_t MACRO_VAR(index) = 0;                                   \
         MACRO_VAR(index) < (1 << (msiSet)->exp); ++MACRO_VAR(index))  \
        if (((element) = (msiSet)->buf[MACRO_VAR(index)]).len != 0)

// Below an example of rehashing with the old set and a growth factor of 0.5.
//
//
// void rehashIndex(MSI_String *oldIndex,
//                  MSI_String *newIndex) {
//     ASSERT(newIndex->len == 0);
//     for (int32_t i = 0; i < (1 << oldIndex->exp); i++) {
//         string s = oldIndex->buf[i];
//         if (s.len > 0) {
//             indexInsert(s, hashStringDjb2(s), newIndex);
//         }
//     }
// }
//
// HashEntry indexInsert(string string, string_HashIndex *index,
//                           arena *perm) {
//     if ((uint32_t)index->len >= ((uint32_t)1 << index->exp) / 2) {
//         string_HashIndex newIndex =
//             (string_HashIndex){.exp = index->exp + 1};
//         newMSISet(&newIndex, SIZEOF(*newIndex.buf),
//                       ALIGNOF(*newIndex.buf), perm);
//         rehashIndex(index, &newIndex);
//         *index = newIndex;
//     }
//     return indexInsert(string, hashStringDjb2(string), index);
// }

#ifdef __cplusplus
}
#endif

#endif
