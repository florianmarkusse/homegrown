#ifndef UTIL_HASH_TRIE_STRING_UINT16_MAP_H
#define UTIL_HASH_TRIE_STRING_UINT16_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for FLO_TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for FLO_MACRO_VAR
#include "shared/allocator/arena.h" // for Arena
#include "util/text/string.h"  // for string
#include <stdint.h>            // for U16

typedef struct {
    string key;
    U16 value;
} flo_trie_StringUint16Data;

typedef struct flo_trie_StringUint16Map {
    struct flo_trie_StringUint16Map *child[4];
    flo_trie_StringUint16Data data;
} flo_trie_StringUint16Map;

U16 flo_trie_insertStringUint16Map(string key, U16 value,
                                        flo_trie_StringUint16Map **set,
                                        Arena *perm);

FLO_TRIE_ITERATOR_HEADER_FILE(flo_trie_StringUint16Map,
                              flo_trie_StringUint16IterNode,
                              flo_trie_StringUint16Iterator,
                              flo_trie_StringUint16Data,
                              flo_createStringUint16Iterator,
                              flo_nextStringUint16Iterator);

#define FLO_FOR_EACH_TRIE_STRING_UINT16(element, stringUint16Map, scratch)     \
    for (flo_trie_StringUint16Iterator * FLO_MACRO_VAR(iter) =                 \
             flo_createStringUint16Iterator(stringUint16Map, &(scratch));      \
         ;)                                                                    \
        if (((element) = flo_nextStringUint16Iterator(FLO_MACRO_VAR(iter),     \
                                                      &(scratch)))             \
                .value == 0)                                                   \
            break;                                                             \
        else

#ifdef __cplusplus
}
#endif

#endif
