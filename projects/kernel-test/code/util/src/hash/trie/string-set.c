#include "util/hash/trie/string-set.h"
#include "util/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for ZERO_MEMORY
#include <stddef.h>                         // for NULL
#include <stdint.h>                         // for uint64_t

bool trie_insertStringSet(string key, trie_StringSet **set, Arena *perm) {
    ASSERT(key.len > 0);
    for (uint64_t hash = hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data)) {
            return false;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, trie_StringSet, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_StringSet, trie_StringIterNode,
                          trie_StringIterator, string, createStringIterator,
                          nextStringIterator);
