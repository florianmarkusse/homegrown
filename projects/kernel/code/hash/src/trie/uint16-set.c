#include "hash/hashes.h"               // for hash16_xm3
#include "hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "hash/trie/u16-set.h"
#include "memory/management/allocator/macros.h"
#include "util/assert.h" // for ASSERT

bool trie_insertU16Set(U16 key, trie_U16Set **set, Arena *perm) {
    ASSERT(key != 0);
    for (U16 hash = hashU16(key); *set != NULL; (hash = (U16)(hash << 2))) {
        if (key == (*set)->data) {
            return false;
        }
        set = &(*set)->child[hash >> 14];
    }
    *set = NEW(perm, trie_U16Set, 1, ZERO_MEMORY);
    (*set)->data = key;
    return true;
}

TRIE_ITERATOR_SOURCE_FILE(trie_U16Set, trie_U16IterNode, trie_U16Iterator, U16,
                          createU16Iterator, nextU16Iterator);
