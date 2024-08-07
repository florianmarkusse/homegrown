#ifndef UTIL_HASH_TRIE_STRING_AUTO_UINT16_MAP_H
#define UTIL_HASH_TRIE_STRING_AUTO_UINT16_MAP_H

#include "common-iterator.h"   // for FLO_TRIE_ITERATOR_HEADER_FILE
#include "util/memory/arena.h" // for flo_arena
#include "util/text/string.h"  // for flo_string
#include <stdint.h>            // for uint16_t

typedef struct {
    flo_string key;
    uint16_t value;
} flo_trie_StringAutoUint16Data;

typedef struct flo_trie_StringAutoUint16Node flo_trie_StringAutoUint16Node;
struct flo_trie_StringAutoUint16Node {
    struct flo_trie_StringAutoUint16Node *child[4];
    flo_trie_StringAutoUint16Data data;
};

typedef struct {
    uint16_t identity;
    flo_trie_StringAutoUint16Node *node;
} flo_trie_StringAutoUint16Map;

typedef struct {
    bool wasInserted;
    uint16_t entryIndex;
} flo_NewStringInsert;

flo_NewStringInsert flo_trie_insertStringAutoUint16Map(
    flo_string key, flo_trie_StringAutoUint16Map *set, flo_arena *perm);

uint16_t
flo_trie_containsStringAutoUint16Map(flo_string key,
                                     flo_trie_StringAutoUint16Map *set);

FLO_TRIE_ITERATOR_HEADER_FILE(flo_trie_StringAutoUint16Node,
                              flo_trie_StringAutoUint16IterNode,
                              flo_trie_StringAutoUint16Iterator,
                              flo_trie_StringAutoUint16Data,
                              flo_createStringAutoUint16Iterator,
                              flo_nextStringAutoUint16Iterator);

#define FLO_FOR_EACH_TRIE_STRING_AUTO_UINT16(element, stringAutoUint16Map,     \
                                             scratch)                          \
    for (flo_trie_StringAutoUint16Iterator *iter =                             \
             flo_createStringAutoUint16Iterator(stringAutoUint16Map,           \
                                                &(scratch));                   \
         ;)                                                                    \
        if (((element) = flo_nextStringAutoUint16Iterator(iter, &(scratch)))   \
                .value == 0)                                                   \
            break;                                                             \
        else

#endif
