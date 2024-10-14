#ifndef UTIL_HASH_TRIE_UINT16_SET_H
#define UTIL_HASH_TRIE_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for FLO_TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for FLO_MACRO_VAR
#include "shared/memory/allocator/arena.h" // for Arena
#include <stdint.h>            // for U16

typedef struct flo_trie_Uint16Set flo_trie_Uint16Set;
struct flo_trie_Uint16Set {
    struct flo_trie_Uint16Set *child[4];
    U16 data;
};

bool flo_trie_insertUint16Set(U16 key, flo_trie_Uint16Set **set,
                              Arena *perm);

FLO_TRIE_ITERATOR_HEADER_FILE(flo_trie_Uint16Set, flo_trie_Uint16IterNode,
                              flo_trie_Uint16Iterator, U16,
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
