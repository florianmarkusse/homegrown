#ifndef UTIL_HASH_TRIE_UINT16_SET_H
#define UTIL_HASH_TRIE_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for MACRO_VAR
#include "util/memory/arena.h" // for arena
#include <stdbool.h>           // for false, true, bool
#include <stdint.h>            // for uint16_t

typedef struct trie_Uint16Set trie_Uint16Set;
struct trie_Uint16Set {
    struct trie_Uint16Set *child[4];
    uint16_t data;
};

bool trie_insertUint16Set(uint16_t key, trie_Uint16Set **set, Arena *perm);

TRIE_ITERATOR_HEADER_FILE(trie_Uint16Set, trie_Uint16IterNode,
                          trie_Uint16Iterator, uint16_t, createUint16Iterator,
                          nextUint16Iterator);

#define FOR_EACH_TRIE_UINT16(element, intSet, scratch)                         \
    for (trie_Uint16Iterator * MACRO_VAR(iter) =                               \
             createUint16Iterator(intSet, &(scratch));                         \
         ;)                                                                    \
        if (((element) = nextUint16Iterator(MACRO_VAR(iter), &(scratch))) ==   \
            0)                                                                 \
            break;                                                             \
        else

#ifdef __cplusplus
}
#endif

#endif
