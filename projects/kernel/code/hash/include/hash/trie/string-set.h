#ifndef UTIL_HASH_TRIE_STRING_SET_H
#define UTIL_HASH_TRIE_STRING_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "memory/management/allocator/arena.h"
#include "util/macros.h"      // for MACRO_VAR
#include "text/string.h" // for string

typedef struct trie_stringSet trie_stringSet;
struct trie_stringSet {
    struct trie_stringSet *child[4];
    string data;
};

bool trie_insertStringSet(string key, trie_stringSet **set, Arena *perm);

TRIE_ITERATOR_HEADER_FILE(trie_stringSet, trie_stringIterNode,
                          trie_stringIterator, string, createStringIterator,
                          nextStringIterator);

#define FOR_EACH_TRIE_STRING(element, stringSet, scratch)                      \
    for (trie_stringIterator * MACRO_VAR(iter) =                               \
             createStringIterator(stringSet, &(scratch));                      \
         ;)                                                                    \
        if (((element) = nextStringIterator(MACRO_VAR(iter), &(scratch)))      \
                .len == 0)                                                     \
            break;                                                             \
        else

// TRIE_ITER_NODE(trie_StringSet, trie_IterNode);

// typedef struct trie_IterNode trie_IterNode;
// struct trie_IterNode {
//     trie_IterNode *next;
//     trie_StringSet *set;
//     unsigned char index; // The branch rate of the trie.
// };

// TRIE_ITERATOR(trie_IterNode, trie_Iter);

// typedef struct {
//     trie_IterNode *head;
//     trie_IterNode *free;
// } trie_Iter;

// TRIE_NEW_ITERATOR(trie_StringSet, trie_Iter, trie_IterNode,
// newIter);

//__attribute((unused)) static trie_Iter *newIter(trie_StringSet
//*set,
//                                                        arena *perm) {
//    trie_Iter *it = NEW(perm, trie_Iter, 1, ZERO_MEMORY);
//    if (set != NULL) {
//        it->head = NEW(perm, trie_IterNode, 1, ZERO_MEMORY);
//        it->head->set = set;
//    }
//    return it;
//}

// TRIE_NEXT_ITERATOR(string, trie_Iter, trie_IterNode,
// nextIter);

//__attribute((unused)) static string nextIter(trie_Iter *it,
//                                                     arena *perm) {
//    while (it->head) {
//        int index = it->head->index++;
//        if (index == 0) {
//            return it->head->set->key;
//        } else if (index == 5) {
//            trie_IterNode *dead =
//                it->head; // Dead node is put on free list, to be reused if
//                need
//                          // be.
//            it->head = dead->next;
//            dead->next = it->free;
//            it->free = dead;
//        } else if (it->head->set->child[index - 1]) {
//            trie_IterNode *nextIter = it->free;
//            if (nextIter != NULL) {
//                it->free = it->free->next;
//                nextIter->index = 0;
//            } else {
//                nextIter = NEW(perm, trie_IterNode, 1,
//                ZERO_MEMORY);
//            }
//            nextIter->set = it->head->set->child[index - 1];
//            nextIter->next = it->head;
//            it->head = nextIter;
//        }
//    }
//    return (string){0};
//}

#ifdef __cplusplus
}
#endif

#endif
