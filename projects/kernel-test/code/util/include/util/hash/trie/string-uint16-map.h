#ifndef UTIL_HASH_TRIE_STRING_UINT16_MAP_H
#define UTIL_HASH_TRIE_STRING_UINT16_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for MACRO_VAR
#include "util/memory/arena.h" // for arena
#include "util/text/string.h"  // for string
#include <stdint.h>            // for uint16_t

typedef struct {
    string key;
    uint16_t value;
} trie_StringUint16Data;

typedef struct trie_StringUint16Map {
    struct trie_StringUint16Map *child[4];
    trie_StringUint16Data data;
} trie_StringUint16Map;

uint16_t trie_insertStringUint16Map(string key, uint16_t value,
                                    trie_StringUint16Map **set, Arena *perm);

TRIE_ITERATOR_HEADER_FILE(trie_StringUint16Map, trie_StringUint16IterNode,
                          trie_StringUint16Iterator, trie_StringUint16Data,
                          createStringUint16Iterator, nextStringUint16Iterator);

#define FOR_EACH_TRIE_STRING_UINT16(element, stringUint16Map, scratch)         \
    for (trie_StringUint16Iterator * MACRO_VAR(iter) =                         \
             createStringUint16Iterator(stringUint16Map, &(scratch));          \
         ;)                                                                    \
        if (((element) =                                                       \
                 nextStringUint16Iterator(MACRO_VAR(iter), &(scratch)))        \
                .value == 0)                                                   \
            break;                                                             \
        else

#ifdef __cplusplus
}
#endif

#endif
