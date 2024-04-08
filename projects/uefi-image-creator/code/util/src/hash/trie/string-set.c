#include "util/hash/trie/string-set.h"
#include "util/assert.h"                    // for FLO_ASSERT
#include "util/hash/hashes.h"               // for flo_hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for FLO_TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for FLO_ZERO_MEMORY
#include <stddef.h>                         // for NULL
#include <stdint.h>                         // for uint64_t

bool flo_trie_insertStringSet(flo_string key, flo_trie_StringSet **set,
                              flo_arena *perm) {
    FLO_ASSERT(key.len > 0);
    for (uint64_t hash = flo_hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (flo_stringEquals(key, (*set)->data)) {
            return false;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = FLO_NEW(perm, flo_trie_StringSet, 1, FLO_ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

FLO_TRIE_ITERATOR_SOURCE_FILE(flo_trie_StringSet, flo_trie_StringIterNode,
                              flo_trie_StringIterator, flo_string,
                              flo_createStringIterator, flo_nextStringIterator);
