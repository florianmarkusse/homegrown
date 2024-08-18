#include "hash/trie/string-u16-map.h"
#include "hash/hashes.h"               // for hashStringDjb2
#include "hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "interoperation/types.h"
#include "memory/management/allocator/macros.h"
#include "util/assert.h" // for ASSERT

U16 trie_insertStringU16Map(string key, U16 value, trie_stringU16Map **set,
                            Arena *perm) {
    ASSERT(key.len > 0);
    ASSERT(value != 0);
    for (U64 hash = hashStringSkeeto(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data.key)) {
            return (*set)->data.value;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, trie_stringU16Map, 1, ZERO_MEMORY);
    (*set)->data.key = key;
    (*set)->data.value = value;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_stringU16Map, trie_stringU16IterNode,
                          trie_stringU16Iterator, trie_stringU16Data,
                          createStringU16Iterator, nextStringU16Iterator);