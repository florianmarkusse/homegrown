#ifndef UTIL_HASH_TRIE_STRING_AUTO_UINT16_MAP_H
#define UTIL_HASH_TRIE_STRING_AUTO_UINT16_MAP_H

#include "common-iterator.h"   // for TRIE_ITERATOR_HEADER_FILE
#include "util/memory/arena.h" // for arena
#include "util/text/string.h"  // for string
#include <stdbool.h>           // for false, true, bool
#include <stdint.h>            // for uint16_t

typedef struct {
    string key;
    uint16_t value;
} trie_StringAutoUint16Data;

typedef struct trie_StringAutoUint16Node trie_StringAutoUint16Node;
struct trie_StringAutoUint16Node {
    struct trie_StringAutoUint16Node *child[4];
    trie_StringAutoUint16Data data;
};

typedef struct {
    uint16_t identity;
    trie_StringAutoUint16Node *node;
} trie_StringAutoUint16Map;

typedef struct {
    bool wasInserted;
    uint16_t entryIndex;
} NewStringInsert;

NewStringInsert trie_insertStringAutoUint16Map(string key,
                                               trie_StringAutoUint16Map *set,
                                               Arena *perm);

uint16_t trie_containsStringAutoUint16Map(string key,
                                          trie_StringAutoUint16Map *set);

TRIE_ITERATOR_HEADER_FILE(trie_StringAutoUint16Node,
                          trie_StringAutoUint16IterNode,
                          trie_StringAutoUint16Iterator,
                          trie_StringAutoUint16Data,
                          createStringAutoUint16Iterator,
                          nextStringAutoUint16Iterator);

#define FOR_EACH_TRIE_STRING_AUTO_UINT16(element, stringAutoUint16Map,         \
                                         scratch)                              \
    for (trie_StringAutoUint16Iterator *iter =                                 \
             createStringAutoUint16Iterator(stringAutoUint16Map, &(scratch));  \
         ;)                                                                    \
        if (((element) = nextStringAutoUint16Iterator(iter, &(scratch)))       \
                .value == 0)                                                   \
            break;                                                             \
        else

#endif
