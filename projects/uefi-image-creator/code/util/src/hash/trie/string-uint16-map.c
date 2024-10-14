#include "util/hash/trie/string-uint16-map.h"
#include "interoperation/assert.h"                    // for ASSERT
#include "util/hash/hashes.h"               // for flo_hashStringDjb2
#include "util/hash/trie/common-iterator.h" // for FLO_TRIE_ITERATOR_SOURCE...
#include "shared/memory/allocator/macros.h"             // for ZERO_MEMORY
#include <stddef.h>                         // for NULL

U16 flo_trie_insertStringUint16Map(string key, U16 value,
                                        flo_trie_StringUint16Map **set,
                                        Arena *perm) {
    ASSERT(key.len > 0);
    ASSERT(value != 0);
    for (U64 hash = flo_hashStringDjb2(key); *set != NULL; hash <<= 2) {
        if (stringEquals(key, (*set)->data.key)) {
            return (*set)->data.value;
        }
        set = &(*set)->child[hash >> 62];
    }
    *set = NEW(perm, flo_trie_StringUint16Map, 1, ZERO_MEMORY);
    (*set)->data.key = key;
    (*set)->data.value = value;
    return true;
}

FLO_TRIE_ITERATOR_SOURCE_FILE(flo_trie_StringUint16Map,
                              flo_trie_StringUint16IterNode,
                              flo_trie_StringUint16Iterator,
                              flo_trie_StringUint16Data,
                              flo_createStringUint16Iterator,
                              flo_nextStringUint16Iterator);
