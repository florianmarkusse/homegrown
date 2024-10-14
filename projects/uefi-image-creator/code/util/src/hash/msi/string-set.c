#include "util/hash/msi/string-set.h"
#include "util/hash/hashes.h"     // for flo_hashStringDjb2
#include "util/hash/msi/common.h" // for flo_indexLookup
#include <stdint.h>               // for int32_t

bool flo_msi_insertString(string string, size_t hash,
                          flo_msi_String *index) {
    for (int32_t i = (int32_t)hash;;) {
        i = flo_indexLookup(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            index->len++;
            index->buf[i] = string;
            return true;
        } else if (stringEquals(index->buf[i], string)) {
            return false;
        }
    }
}

bool flo_msi_containsString(string string, size_t hash,
                            flo_msi_String *index) {
    for (int32_t i = (int32_t)hash;;) {
        i = flo_indexLookup(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            return false;
        } else if (stringEquals(index->buf[i], string)) {
            return true;
        }
    }
}

flo_HashComparisonStatus flo_msi_equalsStringSet(flo_msi_String *set1,
                                                 flo_msi_String *set2) {
    if (set1->len != set2->len) {
        return HASH_COMPARISON_DIFFERENT_SIZES;
    }

    string element;
    FLO_FOR_EACH_MSI_STRING(element, set1) {
        if (!flo_msi_containsString(element, flo_hashStringDjb2(element),
                                    set2)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    FLO_FOR_EACH_MSI_STRING(element, set2) {
        if (!flo_msi_containsString(element, flo_hashStringDjb2(element),
                                    set1)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    return HASH_COMPARISON_SUCCESS;
}
