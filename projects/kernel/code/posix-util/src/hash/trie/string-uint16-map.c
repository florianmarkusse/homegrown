#include "util/hash/trie/string-uint16-map.h"
#include "util/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for ZERO_MEMORY
#include <stdbool.h>                        // for true
#include <stddef.h>                         // for NULL

uint16_t trie_insertStringUint16Map(string key, uint16_t value,
                                    trie_StringUint16Map **set, Arena *perm) {
    ASSERT(key.len > 0);
    ASSERT(value != 0);
    for (uint64_t hash = hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data.key)) {
            return (*set)->data.value;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, trie_StringUint16Map, 1, ZERO_MEMORY);
    (*set)->data.key = key;
    (*set)->data.value = value;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_StringUint16Map, trie_StringUint16IterNode,
                          trie_StringUint16Iterator, trie_StringUint16Data,
                          createStringUint16Iterator, nextStringUint16Iterator);
