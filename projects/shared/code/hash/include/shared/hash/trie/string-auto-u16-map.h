#ifndef HASH_TRIE_STRING_AUTO_U16_MAP_H
#define HASH_TRIE_STRING_AUTO_U16_MAP_H

#include "common-iterator.h" // for TRIE_ITERATOR_HEADER_FILE
#include "interoperation/types.h"
#include "shared/memory/allocator/arena.h"
#include "shared/text/string.h" // for string

typedef struct {
    string key;
    U16 value;
} trie_stringAutoU16Data;

typedef struct trie_stringAutoU16Node trie_stringAutoU16Node;
struct trie_stringAutoU16Node {
    struct trie_stringAutoU16Node *child[4];
    trie_stringAutoU16Data data;
};

typedef struct {
    // Used as the auto-incrementing key
    U16 nodeCount;
    trie_stringAutoU16Node *node;
} trie_stringAutoU16Map;

typedef struct {
    bool wasInserted;
    U16 entryIndex;
} NewStringInsert;

NewStringInsert trie_insertStringAutoU16Map(string key,
                                            trie_stringAutoU16Map *set,
                                            Arena *perm);

U16 trie_containsStringAutoU16Map(string key, trie_stringAutoU16Map *set);

TRIE_ITERATOR_HEADER_FILE(trie_stringAutoU16Node, trie_stringAutoU16IterNode,
                          trie_stringAutoU16Iterator, trie_stringAutoU16Data,
                          createStringAutoU16Iterator,
                          nextStringAutoU16Iterator);

#define FOR_EACH_TRIE_STRING_AUTO_U16(element, stringAutoU16Map, scratch)      \
    for (trie_stringAutoU16Iterator *iter =                                    \
             createStringAutoU16Iterator(stringAutoU16Map, &(scratch));        \
         ;)                                                                    \
        if (((element) = nextStringAutoU16Iterator(iter, &(scratch))).value == \
            0)                                                                 \
            break;                                                             \
        else

#endif
