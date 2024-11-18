#include "shared/hash/trie/string-auto-u16-map.h"
#include "shared/assert.h"      // for ASSERT
#include "shared/hash/hashes.h"               // for hashStringDjb2
#include "shared/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"

NewStringInsert trie_insertStringAutoU16Map(string key,
                                            trie_stringAutoU16Map *set,
                                            Arena *perm) {
    trie_stringAutoU16Node **currentNode = &set->node;
    for (U64 hash = hashStringSkeeto(key); *currentNode != NULL; hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (NewStringInsert){.entryIndex = (*currentNode)->data.value,
                                     .wasInserted = false};
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    if (set->nodeCount == U16_MAX) {
        ASSERT(false);
        __builtin_longjmp(perm->jmp_buf, 1);
    }
    *currentNode = NEW(perm, trie_stringAutoU16Node, 1, ZERO_MEMORY);
    (*currentNode)->data.key = key;
    set->nodeCount++;
    (*currentNode)->data.value = set->nodeCount;
    return (NewStringInsert){.wasInserted = true,
                             .entryIndex = (set)->nodeCount};
}

U16 trie_containsStringAutoU16Map(string key, trie_stringAutoU16Map *set) {
    trie_stringAutoU16Node **currentNode = &set->node;
    for (U64 hash = hashStringSkeeto(key); *currentNode != NULL; hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (*currentNode)->data.value;
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    return 0;
}

TRIE_ITERATOR_SOURCE_FILE(trie_stringAutoU16Node, trie_stringAutoU16IterNode,
                          trie_stringAutoU16Iterator, trie_stringAutoU16Data,
                          createStringAutoU16Iterator,
                          nextStringAutoU16Iterator);
