#include "util/hash/trie/uint16-set.h"
#include "interoperation/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for flo_hash16_xm3
#include "util/hash/trie/common-iterator.h" // for FLO_TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"             // for ZERO_MEMORY
#include <stddef.h>                         // for NULL

bool flo_trie_insertUint16Set(U16 key, flo_trie_Uint16Set **set,
                              Arena *perm) {
    ASSERT(key != 0);
    for (U16 hash = flo_hash16_xm3(key); *set != NULL;
         (hash = (U16)(hash << 2))) {
        if (key == (*set)->data) {
            return false;
        }
        set = &(*set)->child[hash >> 14];
    }
    *set = NEW(perm, flo_trie_Uint16Set, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

FLO_TRIE_ITERATOR_SOURCE_FILE(flo_trie_Uint16Set, flo_trie_Uint16IterNode,
                              flo_trie_Uint16Iterator, U16,
                              flo_createUint16Iterator, flo_nextUint16Iterator);
