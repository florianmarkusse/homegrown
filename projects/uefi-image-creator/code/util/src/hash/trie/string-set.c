#include "util/hash/trie/string-set.h"
#include "interoperation/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for flo_hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for FLO_TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"             // for ZERO_MEMORY
#include <stddef.h>                         // for NULL
#include <stdint.h>                         // for U64

bool flo_trie_insertStringSet(string key, flo_trie_StringSet **set,
                              Arena *perm) {
    ASSERT(key.len > 0);
    for (U64 hash = flo_hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data)) {
            return false;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, flo_trie_StringSet, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

FLO_TRIE_ITERATOR_SOURCE_FILE(flo_trie_StringSet, flo_trie_StringIterNode,
                              flo_trie_StringIterator, string,
                              flo_createStringIterator, flo_nextStringIterator);
