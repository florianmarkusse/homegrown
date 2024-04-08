#include "util/hash/trie/string-uint16-map.h"
#include "util/assert.h"                    // for FLO_ASSERT
#include "util/hash/hashes.h"               // for flo_hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for FLO_TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for FLO_ZERO_MEMORY
#include <stdbool.h>                        // for true
#include <stddef.h>                         // for NULL

uint16_t flo_trie_insertStringUint16Map(flo_string key, uint16_t value,
                                        flo_trie_StringUint16Map **set,
                                        flo_arena *perm) {
    FLO_ASSERT(key.len > 0);
    FLO_ASSERT(value != 0);
    for (uint64_t hash = flo_hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (flo_stringEquals(key, (*set)->data.key)) {
            return (*set)->data.value;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = FLO_NEW(perm, flo_trie_StringUint16Map, 1, FLO_ZERO_MEMORY);
    (*set)->data.key = key;
    (*set)->data.value = value;
    return true;
}

FLO_TRIE_ITERATOR_SOURCE_FILE(flo_trie_StringUint16Map,
                              flo_trie_StringUint16IterNode,
                              flo_trie_StringUint16Iterator,
                              flo_trie_StringUint16Data,
                              flo_createStringUint16Iterator,
                              flo_nextStringUint16Iterator);
