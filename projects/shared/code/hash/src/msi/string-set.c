#include "hash/msi/string-set.h"
#include "hash/hashes.h"     // for hashStringDjb2
#include "hash/msi/common.h" // for indexLookup

bool msi_insertString(string string, U64 hash, msi_string *index) {
    for (U32 i = (U32)hash;;) {
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

bool msi_containsString(string string, U64 hash, msi_string *index) {
    for (U32 i = (U32)hash;;) {
        i = indexLookup(hash, index->exp, i);
        if (index->buf[i].len == 0) {
            return false;
        } else if (stringEquals(index->buf[i], string)) {
            return true;
        }
    }
}

HashComparisonStatus msi_equalsStringSet(msi_string *set1, msi_string *set2) {
    if (set1->len != set2->len) {
        return HASH_COMPARISON_DIFFERENT_SIZES;
    }

    string element;
    FOR_EACH_MSI_STRING(element, set1) {
        if (!msi_containsString(element, hashStringSkeeto(element), set2)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    FOR_EACH_MSI_STRING(element, set2) {
        if (!msi_containsString(element, hashStringSkeeto(element), set1)) {
            return HASH_COMPARISON_DIFFERENT_CONTENT;
        }
    }

    return HASH_COMPARISON_SUCCESS;
}
