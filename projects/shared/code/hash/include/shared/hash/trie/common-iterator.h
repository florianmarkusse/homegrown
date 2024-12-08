#ifndef SHARED_HASH_TRIE_COMMON_ITERATOR_H
#define SHARED_HASH_TRIE_COMMON_ITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ugly code ahead. Rewriting this iterator each time is a massive pain so I
 * decided to write this grotesque set of macros to automatically build it for
 * me. It looks crazy but works surprisingly well.
 */

#define TRIE_ITER_NODE(T, iterNodeName)                                        \
    typedef struct iterNodeName iterNodeName;                                  \
    struct iterNodeName {                                                      \
        /* NOLINTNEXTLINE */                                                   \
        iterNodeName *next;                                                    \
        /* NOLINTNEXTLINE */                                                   \
        T *set;                                                                \
        unsigned char index;                                                   \
    };

#define TRIE_ITERATOR(iterNodeType, iteratorName)                              \
    typedef struct {                                                           \
        /* NOLINTNEXTLINE */                                                   \
        iterNodeType *head;                                                    \
        /* NOLINTNEXTLINE */                                                   \
        iterNodeType *free;                                                    \
    }(iteratorName);

#define TRIE_ITERATOR_HEADER_FILE(setType, iterNodeType, iteratorType,         \
                                  returnType, createIteratorFunctionName,      \
                                  nextIteratorFunctionName)                    \
    TRIE_ITER_NODE(setType, iterNodeType);                                     \
    TRIE_ITERATOR(iterNodeType, iteratorType);                                 \
    /* NOLINTNEXTLINE */                                                       \
    iteratorType *createIteratorFunctionName(/* NOLINTNEXTLINE */              \
                                             setType *set, Arena *perm);       \
    returnType /* NOLINTNEXTLINE */                                            \
    nextIteratorFunctionName(iteratorType *it, Arena *perm);

#define TRIE_NEW_ITERATOR(stringSetType, iteratorType, iterNodeType,           \
                          functionName)                                        \
    /* NOLINTNEXTLINE */                                                       \
    iteratorType *functionName(/* NOLINTNEXTLINE */                            \
                               stringSetType *set, Arena *perm) {              \
        /* NOLINTNEXTLINE */                                                   \
        iteratorType *it = NEW(perm, iteratorType, 1, ZERO_MEMORY);            \
        if (set != NULL) {                                                     \
            it->head = NEW(perm, iterNodeType, 1, ZERO_MEMORY);                \
            it->head->set = set;                                               \
        }                                                                      \
        return it;                                                             \
    }

#define TRIE_NEXT_ITERATOR(returnType, iteratorType, iterNodeType,             \
                           functionName)                                       \
    /* NOLINTNEXTLINE */                                                       \
    returnType functionName(iteratorType *it, Arena *perm) {                   \
        while (it->head) {                                                     \
            int index = it->head->index++;                                     \
            if (index == 0) {                                                  \
                return it->head->set->data;                                    \
            } else if (index == 5) {                                           \
                /* NOLINTNEXTLINE */                                           \
                iterNodeType *dead = it->head;                                 \
                it->head = dead->next;                                         \
                dead->next = it->free;                                         \
                it->free = dead;                                               \
            } else if (it->head->set->child[index - 1]) {                      \
                /* NOLINTNEXTLINE */                                           \
                iterNodeType *nextIter = it->free;                             \
                if (nextIter != NULL) {                                        \
                    it->free = it->free->next;                                 \
                    nextIter->index = 0;                                       \
                } else {                                                       \
                    nextIter = NEW(perm, iterNodeType, 1, ZERO_MEMORY);        \
                }                                                              \
                nextIter->set = it->head->set->child[index - 1];               \
                nextIter->next = it->head;                                     \
                it->head = nextIter;                                           \
            }                                                                  \
        }                                                                      \
        return (returnType){0};                                                \
    }

#define TRIE_ITERATOR_SOURCE_FILE(setType, iterNodeType, iteratorType,         \
                                  returnType, createIteratorFunctionName,      \
                                  nextIteratorFunctionName)                    \
    TRIE_NEW_ITERATOR(setType, iteratorType, iterNodeType,                     \
                      createIteratorFunctionName)                              \
    TRIE_NEXT_ITERATOR(returnType, iteratorType, iterNodeType,                 \
                       nextIteratorFunctionName)

#ifdef __cplusplus
}
#endif

#endif
