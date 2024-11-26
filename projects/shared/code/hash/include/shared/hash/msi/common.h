#ifndef SHARED_HASH_MSI_COMMON_H
#define SHARED_HASH_MSI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shared/macros.h" // for MACRO_VAR
#include "shared/types/types.h"
#include "shared/memory/allocator/arena.h"
#include "shared/memory/allocator/macros.h"

/**
 * Common definitions for MSI string hash.
 * https://nullprogram.com/blog/2022/08/08/
 *
 * This is a double hashed, open address hash table. One can easily add a
 * hashmap on top of this if the need arises.
 * For example, instead of creating a MSI_SET of string, create one of
 * {
 *  string key;
 *  ValueType {
 *    ...
 *  }
 * }
 * and create a contains function that returns this struct or the value
 * directly.
 *
 * Note that this is not super
 * friendly to using arenas. Resizing is supported but using the hash-trie
 * implementation is preferred.
 *
 * Support for the following features (may need to
 * be implemented manually):
 * - Insertion / contains checks
 * - Retrieving the index of the string of the underlaying hash table.
 * - Deletion can be implemented by means of a tombstone, for example.
 *
 * The caller has to take care of the following:
 * - The hashing of the key. See hashes.h file for common hashes.
 * - Ensuring the set has enough size to add another entry into the set, this
 * includes the growth factor.
 * - How to rehash the hash set if the capacity is exceeded. For example,
 *   - By going over the values of the backing buffer
 *   - By allocating another set and rehashing the values of the old set into
 * the new set.
 */

#define MSI_SET(T)                                                             \
    struct {                                                                   \
        T *buf;                                                                \
        U8 exp;                                                                \
        U64 len;                                                               \
    }

typedef MSI_SET(U8) SetSlice;

#define NEW_MSI_SET(T, exponent, perm)                                         \
    ({                                                                         \
        T MACRO_VAR(newSet) = (T){.exp = (exponent)};                          \
        msi_newSet(&MACRO_VAR(newSet), sizeof(*MACRO_VAR(newSet).buf),         \
                   alignof(*MACRO_VAR(newSet).buf), perm);                     \
        MACRO_VAR(newSet);                                                     \
    })

// If this ever changes types because it's too small, make sure to test out that
// it works.
// This return typa used to be an I32, but U32 "should" be okay
static inline U32 indexLookup(U64 hash, U16 exp, U32 idx) {
    U32 mask = ((U32)1 << exp) - 1;
    U32 step = (U32)(hash >> (64 - exp)) | 1;
    return (idx + step) & mask;
}

void msi_newSet(void *setSlice, U64 size, U64 align, Arena *a);

#ifdef __cplusplus
}
#endif

#endif
