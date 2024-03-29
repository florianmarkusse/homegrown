#ifndef UTIL_HASH_MSI_COMMON_H
#define UTIL_HASH_MSI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/macros.h"        // for FLO_MACRO_VAR
#include "util/memory/arena.h"  // for flo_arena
#include "util/memory/macros.h" // for FLO_ALIGNOF, FLO_SIZEOF
#include <stddef.h>             // for ptrdiff_t
#include <stdint.h>             // for uint32_t, int32_t, uint64_t

/**
 * Common definitions for MSI string hash.
 * https://nullprogram.com/blog/2022/08/08/
 *
 * This is a double hashed, open address hash table. One can easily add a
 * hashmap on top of this if the need arises.
 * For example, instead of creating a FLO_MSI_SET of string, create one of
 * {
 *  flo_string key;
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

#define FLO_MSI_SET(T)                                                         \
    struct {                                                                   \
        T *buf;                                                                \
        unsigned char exp;                                                     \
        ptrdiff_t len;                                                         \
    }

typedef FLO_MSI_SET(char) SetSlice;

#define FLO_NEW_MSI_SET(T, exponent, perm)                                     \
    ({                                                                         \
        T FLO_MACRO_VAR(newSet) = (T){.exp = (exponent)};                      \
        flo_msi_newSet(&FLO_MACRO_VAR(newSet),                                 \
                       FLO_SIZEOF(*FLO_MACRO_VAR(newSet).buf),                 \
                       FLO_ALIGNOF(*FLO_MACRO_VAR(newSet).buf), perm);         \
        FLO_MACRO_VAR(newSet);                                                 \
    })

// If this ever changes types because it's too small, make sure to test out that
// it works.
__attribute((unused)) static inline int32_t
flo_indexLookup(uint64_t hash, int exp, int32_t idx) {
    uint32_t mask = ((uint32_t)1 << exp) - 1;
    uint32_t step = (uint32_t)(hash >> (64 - exp)) | 1;
    return (idx + step) & mask;
}

void flo_msi_newSet(void *setSlice, ptrdiff_t size, ptrdiff_t align,
                    flo_arena *a);

#ifdef __cplusplus
}
#endif

#endif
