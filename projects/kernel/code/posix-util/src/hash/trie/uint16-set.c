#include "util/hash/trie/uint16-set.h"
#include "util/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for hash16_xm3
#include "util/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for ZERO_MEMORY
#include <stddef.h>                         // for NULL

bool trie_insertUint16Set(uint16_t key, trie_Uint16Set **set, Arena *perm) {
    ASSERT(key != 0);
    for (uint16_t hash = hash16_xm3(key); *set != NULL;
         (hash = (uint16_t)(hash << 2))) {
        if (key == (*set)->data) {
            return false;
        }
        set = &(*set)->child[hash >> 14];
    }
    *set = NEW(perm, trie_Uint16Set, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_Uint16Set, trie_Uint16IterNode,
                          trie_Uint16Iterator, uint16_t, createUint16Iterator,
                          nextUint16Iterator);
