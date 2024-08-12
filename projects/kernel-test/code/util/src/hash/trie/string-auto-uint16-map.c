#include "util/hash/trie/string-auto-uint16-map.h"
#include "util/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for TRIE_ITERATOR_SOURCE...
#include "util/memory/macros.h"             // for ZERO_MEMORY
#include <stdbool.h>                        // for false, true, bool
#include <stddef.h>                         // for NULL

NewStringInsert trie_insertStringAutoUint16Map(string key,
                                               trie_StringAutoUint16Map *set,
                                               Arena *perm) {
    trie_StringAutoUint16Node **currentNode = &set->node;
    for (uint64_t hash = hashStringDjb2(key); *currentNode != NULL;
         hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (NewStringInsert){.entryIndex = (*currentNode)->data.value,
                                     .wasInserted = false};
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    if (set->identity == UINT16_MAX) {
        ASSERT(false);
        __builtin_longjmp(perm->jmp_buf, 1);
    }
    *currentNode = NEW(perm, trie_StringAutoUint16Node, 1, ZERO_MEMORY);
    (*currentNode)->data.key = key;
    (*currentNode)->data.value = ++(set)->identity;
    return (NewStringInsert){.wasInserted = true,
                             .entryIndex = (set)->identity};
}

uint16_t trie_containsStringAutoUint16Map(string key,
                                          trie_StringAutoUint16Map *set) {
    trie_StringAutoUint16Node **currentNode = &set->node;
    for (uint64_t hash = hashStringDjb2(key); *currentNode != NULL;
         hash <<= 2) {
        if (stringEquals(key, (*currentNode)->data.key)) {
            return (*currentNode)->data.value;
        }
        currentNode = &(*currentNode)->child[hash >> 62];
    }
    return 0;
}

TRIE_ITERATOR_SOURCE_FILE(trie_StringAutoUint16Node,
                          trie_StringAutoUint16IterNode,
                          trie_StringAutoUint16Iterator,
                          trie_StringAutoUint16Data,
                          createStringAutoUint16Iterator,
                          nextStringAutoUint16Iterator);
