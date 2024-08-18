#include "util/hash/msi/string-set.h"
#include "util/hash/hashes.h"     // for hashStringDjb2
#include "util/hash/msi/common.h" // for indexLookup
#include <stdint.h>               // for int32_t

bool msi_insertString(string string, size_t hash,
                          msi_String *index) {
    for (int32_t i = (int32_t)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            index->len++;
            index->buf[i] = string;
            return true;
        } else if (stringEquals(index->buf[i], string)) {
            return false;
        }
    }
}

bool msi_containsString(string string, size_t hash,
                            msi_String *index) {
    for (int32_t i = (int32_t)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            return false;
        } else if (stringEquals(index->buf[i], string)) {
            return true;
        }
    }
}

HashComparisonStatus msi_equalsStringSet(msi_String *set1,
                                                 msi_String *set2) {
    if (set1->len != set2->len) {
        return HASH_COMPARISON_DIFFERENT_SIZES;
    }

    string element;
    FOR_EACH_MSI_STRING(element, set1) {
        if (!msi_containsString(element, hashStringDjb2(element),
                                    set2)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    FOR_EACH_MSI_STRING(element, set2) {
        if (!msi_containsString(element, hashStringDjb2(element),
                                    set1)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    return HASH_COMPARISON_SUCCESS;
}
