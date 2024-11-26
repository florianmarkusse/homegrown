#ifndef UTIL_HASH_TRIE_UINT16_SET_H
#define UTIL_HASH_TRIE_UINT16_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "shared/types/types.h"
#include "shared/memory/allocator/arena.h"
#include "shared/macros.h" // for MACRO_VAR

typedef struct trie_U16Set trie_U16Set;
struct trie_U16Set {
    struct trie_U16Set *child[4];
    U16 data;
};

bool trie_insertU16Set(U16 key, trie_U16Set **set, Arena *perm);

TRIE_ITERATOR_HEADER_FILE(trie_U16Set, trie_U16IterNode, trie_U16Iterator, U16,
                          createU16Iterator, nextU16Iterator);

#define FOR_EACH_TRIE_U16(element, intSet, scratch)                            \
    for (trie_U16Iterator * MACRO_VAR(iter) =                                  \
             createU16Iterator(intSet, &(scratch));                            \
         ;)                                                                    \
        if (((element) = nextU16Iterator(MACRO_VAR(iter), &(scratch))) == 0)   \
            break;                                                             \
        else

#ifdef __cplusplus
}
#endif

#endif
