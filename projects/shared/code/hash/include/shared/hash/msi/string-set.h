#ifndef SHARED_HASH_MSI_STRING_SET_H
#define SHARED_HASH_MSI_STRING_SET_H

#include "common.h"                             // for MSI_SET
#include "shared/hash/hash-comparison-status.h" // for HashComparisonStatus
#include "shared/macros.h"                      // for MACRO_VAR
#include "shared/text/string.h"                 // for string
#include "shared/types/types.h"

typedef MSI_SET(string) msi_string;

bool msi_insertString(string string, U64 hash, msi_string *index);

/**
 * Assumes you know what hash function was used in this hash set. If you use the
 * wrong hash, you get wrong answers!!!
 */
bool msi_containsString(string string, U64 hash, msi_string *index);

/**
 * Check if the same string hasher is used to compare as the hash functions that
 * were used to insert.
 */
HashComparisonStatus msi_equalsStringSet(msi_string *set1, msi_string *set2);

#define FOR_EACH_MSI_STRING(element, msiSet)                                   \
    for (U64 MACRO_VAR(index) = 0; MACRO_VAR(index) < (1 << (msiSet)->exp);    \
         ++MACRO_VAR(index))                                                   \
        if (((element) = (msiSet)->buf[MACRO_VAR(index)]).len != 0)

// Below an example of rehashing with the old set and a growth factor of 0.5.
//
//
// void rehashIndex(msi_string *oldIndex,
//                  msi_string *newIndex) {
//     ASSERT(newIndex->len == 0);
//     for (U32 i = 0; i < (1 << oldIndex->exp); i++) {
//         string s = oldIndex->buf[i];
//         if (s.len > 0) {
//             indexInsert(s, hashStringDjb2(s), newIndex);
//         }
//     }
// }
//
// HashEntry indexInsert(string string, string_hashIndex *index,
//                           Arena *perm) {
//     if ((U32)index->len >= ((U32)1 << index->exp) / 2) {
//         string_hashIndex newIndex =
//             (string_hashIndex){.exp = index->exp + 1};
//         newMSISet(&newIndex, sizeof(*newIndex.buf),
//                       alignof(*newIndex.buf), perm);
//         rehashIndex(index, &newIndex);
//         *index = newIndex;
//     }
//     return indexInsert(string, hashStringDjb2(string), index);
// }

#endif
