#ifndef UTIL_HASH_MSI_STRING_SET_H
#define UTIL_HASH_MSI_STRING_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"                           // for MSI_SET
#include "util/hash/hash-comparison-status.h" // for flo_HashComparisonStatus
#include "util/macros.h"                      // for FLO_MACRO_VAR
#include "util/text/string.h"                 // for string

typedef MSI_SET(string) flo_msi_String;

bool flo_msi_insertString(string string, U64 hash, flo_msi_String *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool flo_msi_containsString(string string, U64 hash, flo_msi_String *index);

/**
 * Uses flo_hashStringDjb2 to compare so if any of the sets used a customs
 * hashing function, do not use!!!
 */
flo_HashComparisonStatus flo_msi_equalsStringSet(flo_msi_String *set1,
                                                 flo_msi_String *set2);

#define FLO_FOR_EACH_MSI_STRING(element, msiSet)                               \
    for (U64 FLO_MACRO_VAR(index) = 0;                                         \
         FLO_MACRO_VAR(index) < (1 << (msiSet)->exp); ++FLO_MACRO_VAR(index))  \
        if (((element) = (msiSet)->buf[FLO_MACRO_VAR(index)]).len != 0)

// Below an example of rehashing with the old set and a growth factor of 0.5.
//
//
// void rehashIndex(flo_MSI_String *oldIndex,
//                  flo_MSI_String *newIndex) {
//     ASSERT(newIndex->len == 0);
//     for (int32_t i = 0; i < (1 << oldIndex->exp); i++) {
//         string s = oldIndex->buf[i];
//         if (s.len > 0) {
//             flo_indexInsert(s, flo_hashStringDjb2(s), newIndex);
//         }
//     }
// }
//
// flo_HashEntry indexInsert(string string, string_HashIndex *index,
//                           Arena *perm) {
//     if ((U32)index->len >= ((U32)1 << index->exp) / 2) {
//         string_HashIndex newIndex =
//             (string_HashIndex){.exp = index->exp + 1};
//         flo_newMSISet(&newIndex, sizeof(*newIndex.buf),
//                       alignof(*newIndex.buf), perm);
//         rehashIndex(index, &newIndex);
//         *index = newIndex;
//     }
//     return flo_indexInsert(string, flo_hashStringDjb2(string), index);
// }

#ifdef __cplusplus
}
#endif

#endif
