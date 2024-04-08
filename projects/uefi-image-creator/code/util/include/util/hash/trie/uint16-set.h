#ifndef UTIL_HASH_TRIE_UINT16_SET_H
#define UTIL_HASH_TRIE_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for FLO_TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for FLO_MACRO_VAR
#include "util/memory/arena.h" // for flo_arena
#include <stdbool.h>           // for false, true, bool
#include <stdint.h>            // for uint16_t

typedef struct flo_trie_Uint16Set flo_trie_Uint16Set;
struct flo_trie_Uint16Set {
    struct flo_trie_Uint16Set *child[4];
    uint16_t data;
};

bool flo_trie_insertUint16Set(uint16_t key, flo_trie_Uint16Set **set,
                              flo_arena *perm);

FLO_TRIE_ITERATOR_HEADER_FILE(flo_trie_Uint16Set, flo_trie_Uint16IterNode,
                              flo_trie_Uint16Iterator, uint16_t,
                              flo_createUint16Iterator, flo_nextUint16Iterator);

#define FLO_FOR_EACH_TRIE_UINT16(element, intSet, scratch)                     \
    for (flo_trie_Uint16Iterator * FLO_MACRO_VAR(iter) =                       \
             flo_createUint16Iterator(intSet, &(scratch));                     \
         ;)                                                                    \
        if (((element) = flo_nextUint16Iterator(FLO_MACRO_VAR(iter),           \
                                                &(scratch))) == 0)             \
            break;                                                             \
        else

#ifdef __cplusplus
}
#endif

#endif
