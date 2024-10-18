#include "hash/trie/string-set.h"
#include "hash/hashes.h"               // for hashStringDjb2
#include "hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "interoperation/types.h"
#include "shared/memory/allocator/macros.h"
#include "interoperation/assert.h" // for ASSERT

bool trie_insertStringSet(string key, trie_stringSet **set, Arena *perm) {
    ASSERT(key.len > 0);
    for (U64 hash = hashStringSkeeto(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data)) {
            return false;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, trie_stringSet, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_stringSet, trie_stringIterNode,
                          trie_stringIterator, string, createStringIterator,
                          nextStringIterator);
