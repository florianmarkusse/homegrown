#ifndef UTIL_HASH_TRIE_STRING_SET_H
#define UTIL_HASH_TRIE_STRING_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h"   // for FLO_TRIE_ITERATOR_HEADER_FILE
#include "util/macros.h"       // for FLO_MACRO_VAR
#include "util/memory/arena.h" // for flo_arena
#include "util/text/string.h"  // for flo_string

typedef struct flo_trie_StringSet flo_trie_StringSet;
struct flo_trie_StringSet {
    struct flo_trie_StringSet *child[4];
    flo_string data;
};

bool flo_trie_insertStringSet(flo_string key, flo_trie_StringSet **set,
                              flo_arena *perm);

FLO_TRIE_ITERATOR_HEADER_FILE(flo_trie_StringSet, flo_trie_StringIterNode,
                              flo_trie_StringIterator, flo_string,
                              flo_createStringIterator, flo_nextStringIterator);

#define FLO_FOR_EACH_TRIE_STRING(element, stringSet, scratch)                  \
    for (flo_trie_StringIterator * FLO_MACRO_VAR(iter) =                       \
             flo_createStringIterator(stringSet, &(scratch));                  \
         ;)                                                                    \
        if (((element) =                                                       \
                 flo_nextStringIterator(FLO_MACRO_VAR(iter), &(scratch)))      \
                .len == 0)                                                     \
            break;                                                             \
        else

// FLO_TRIE_ITER_NODE(flo_trie_StringSet, flo_trie_IterNode);

// typedef struct flo_trie_IterNode flo_trie_IterNode;
// struct flo_trie_IterNode {
//     flo_trie_IterNode *next;
//     flo_trie_StringSet *set;
//     unsigned char index; // The branch rate of the trie.
// };

// FLO_TRIE_ITERATOR(flo_trie_IterNode, flo_trie_Iter);

// typedef struct {
//     flo_trie_IterNode *head;
//     flo_trie_IterNode *free;
// } flo_trie_Iter;

// FLO_TRIE_NEW_ITERATOR(flo_trie_StringSet, flo_trie_Iter, flo_trie_IterNode,
// flo_newIter);

//__attribute((unused)) static flo_trie_Iter *flo_newIter(flo_trie_StringSet
//*set,
//                                                        flo_arena *perm) {
//    flo_trie_Iter *it = FLO_NEW(perm, flo_trie_Iter, 1, FLO_ZERO_MEMORY);
//    if (set != NULL) {
//        it->head = FLO_NEW(perm, flo_trie_IterNode, 1, FLO_ZERO_MEMORY);
//        it->head->set = set;
//    }
//    return it;
//}

// FLO_TRIE_NEXT_ITERATOR(flo_string, flo_trie_Iter, flo_trie_IterNode,
// flo_nextIter);

//__attribute((unused)) static flo_string flo_nextIter(flo_trie_Iter *it,
//                                                     flo_arena *perm) {
//    while (it->head) {
//        int index = it->head->index++;
//        if (index == 0) {
//            return it->head->set->key;
//        } else if (index == 5) {
//            flo_trie_IterNode *dead =
//                it->head; // Dead node is put on free list, to be reused if
//                need
//                          // be.
//            it->head = dead->next;
//            dead->next = it->free;
//            it->free = dead;
//        } else if (it->head->set->child[index - 1]) {
//            flo_trie_IterNode *nextIter = it->free;
//            if (nextIter != NULL) {
//                it->free = it->free->next;
//                nextIter->index = 0;
//            } else {
//                nextIter = FLO_NEW(perm, flo_trie_IterNode, 1,
//                FLO_ZERO_MEMORY);
//            }
//            nextIter->set = it->head->set->child[index - 1];
//            nextIter->next = it->head;
//            it->head = nextIter;
//        }
//    }
//    return (flo_string){0};
//}

#ifdef __cplusplus
}
#endif

#endif
